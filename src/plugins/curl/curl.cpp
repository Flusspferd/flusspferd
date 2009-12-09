// -*- mode:c++;coding:utf-8; -*- vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:enc=utf-8:
/*
  The MIT License

  Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
  http://flusspferd.org/contributors.txt)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "flusspferd/create.hpp"
#include "flusspferd/binary.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/modules.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/class_description.hpp"
#include "flusspferd/property_iterator.hpp"

#include <sstream>
#include <curl/curl.h>

#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/assign/ptr_map_inserter.hpp>
#include <boost/any.hpp>

using namespace flusspferd;

namespace {
  namespace bf = boost::fusion;

  typedef boost::error_info<struct tag_curlcode, CURLcode> curlcode_info;

  struct exception
    : flusspferd::exception
  {
    exception(std::string const &what)
      : std::runtime_error(what), flusspferd::exception(what)
    { }

    char const *what() const throw() {
      if(CURLcode const *code = ::boost::get_error_info<curlcode_info>(*this)) {
        std::string what_ = flusspferd::exception::what();
        what_ += ": ";
        what_ += curl_easy_strerror(*code);
        return what_.c_str();
      }
      else {
        return flusspferd::exception::what();
      }
    }
  };

  void global_init(long flags) {
    CURLcode ret = curl_global_init(flags);
    if(ret != 0) {
      throw flusspferd::exception(std::string("curl_global_init: ")
                                  + curl_easy_strerror(ret));
    }
  }

  class Easy;

  namespace {
    struct handle_option {
      virtual ~handle_option() =0;
      virtual function getter() const =0;
      virtual function setter() const =0;
      virtual boost::any data() const =0;
      virtual CURLoption what() const =0;
    };
    handle_option::~handle_option() { }

    typedef boost::ptr_unordered_map<std::string, handle_option> options_map_t;
    options_map_t const &get_options();
  }

  FLUSSPFERD_CLASS_DESCRIPTION
  (
   EasyOpt,
   (constructor_name, "EasyOpt")
   (full_name, "cURL.Easy.EasyOpt")
   (constructible, false)
   )
  {
  public: // TODO
    typedef boost::unordered_map<CURLoption, boost::any> data_map_t;
    data_map_t data;
    Easy &parent;
  public:
    EasyOpt(flusspferd::object const &self, Easy &parent)
      : base_type(self), parent(parent)
    {	}

    static EasyOpt &create(Easy &p) {
      return flusspferd::create<EasyOpt>(bf::make_vector(boost::ref(p)));
    }
  protected:
    bool property_resolve(value const &id, unsigned access);
  };

  FLUSSPFERD_CLASS_DESCRIPTION
  (
   Easy,
   (constructor_name, "Easy")
   (full_name, "cURL.Easy")
   (methods,
    ("cleanup",  bind, cleanup)
    ("perform",  bind, perform)
    ("reset",    bind, reset)
    ("escape",   bind, escape)
    ("unescape", bind, unescape)
    ("valid",    bind, valid))
   (properties,
    ("options", getter, get_opt))
   )
  {
    CURL *handle;
    EasyOpt &opt;

  public: // TODO
    object writefunction_callback;
    static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream) {
      assert(stream);
      Easy &self = *reinterpret_cast<Easy*>(stream);
      if(self.writefunction_callback.is_null()) {
        return 0;
      }
      else {
        byte_array &data = flusspferd::create<byte_array>(
          bf::make_vector(reinterpret_cast<byte_array::element_type*>(ptr),
                          size*nmemb));
        root_object d(data);
        arguments arg;
        arg.push_back(value(data));
        arg.push_back(value(size));
        value v = self.writefunction_callback.call(arg);
        return v.to_number();
      }
    }

    object readfunction_callback;
    static size_t readfunction(void *ptr, size_t size, size_t nmemb, void *stream) {
      assert(stream);
      Easy &self = *reinterpret_cast<Easy*>(stream);
      if(self.readfunction_callback.is_null()) {
        return CURL_READFUNC_ABORT;
      }
      else {
        byte_array &data = flusspferd::create<byte_array>(
          bf::vector2<binary::element_type*, std::size_t>(0x0, 0));
        root_object d(data);
        arguments arg;
        arg.push_back(value(data));
        arg.push_back(value(size));
        arg.push_back(value(nmemb));
        value v = self.readfunction_callback.call(arg);
        if(data.get_length() > size*nmemb) {
          throw flusspferd::exception("Out of Range");
        }
        std::copy(data.get_data().begin(), data.get_data().end(),
                  static_cast<binary::element_type*>(ptr));
        return v.to_number();
      }
    }

    object headerfunction_callback;
    static size_t headerfunction(void *ptr, size_t size, size_t nmemb, void *stream) {
      assert(stream);
      Easy &self = *reinterpret_cast<Easy*>(stream);
      if(self.headerfunction_callback.is_null()) {
        return 0;
      }
      else {
        byte_array &data = flusspferd::create<byte_array>(
          bf::make_vector(reinterpret_cast<byte_array::element_type*>(ptr),
                          size*nmemb));
        root_object d(data);
        arguments arg;
        arg.push_back(value(data));
        arg.push_back(value(size));
        value v = self.headerfunction_callback.call(arg);
        return v.to_number();
      }
    }

    object progressfunction_callback;
    static int progressfunction(
      void *clientp,
      double dltotal, double dlnow,
      double ultotal, double ulnow)
    {
      assert(clientp);
      Easy &self = *reinterpret_cast<Easy*>(clientp);
      if(self.progressfunction_callback.is_null()) {
        return 0;
      }
      else {
        arguments arg;
        arg.push_back(value(dltotal));
        arg.push_back(value(dlnow));
        arg.push_back(value(ultotal));
        arg.push_back(value(ulnow));
        value v = self.progressfunction_callback.call(arg);
        return v.to_number();
      }
    }

    object debugfunction_callback;
    static int debugfunction(CURL *hnd, curl_infotype i, char *buf, size_t len, void *p) {
      assert(p);
      Easy &self = *reinterpret_cast<Easy*>(p);
      if(self.progressfunction_callback.is_null() || hnd != self.handle) {
        return 0;
      }
      else {
        arguments arg;
        byte_array &data = flusspferd::create<byte_array>(
          bf::vector2<binary::element_type*, std::size_t>(
            reinterpret_cast<binary::element_type*>(buf), len));
        root_object d(data);
        arg.push_back(value(static_cast<int>(i)));
        arg.push_back(value(data));
        value v = self.debugfunction_callback.call(arg);
        return v.to_number();
      }
    }

  protected:
    void trace(flusspferd::tracer &trc) {
      trc("options", opt);
      trc("writeFunction", writefunction_callback);
      trc("readFunction", readfunction_callback);
      trc("progressFunction", progressfunction_callback);
      trc("headerFunction", headerfunction_callback);
      trc("debugFunction", debugfunction_callback);
    }

  public:
    CURL *data() { return handle; }
    bool valid() { return handle; }
    CURL *get() {
      if(!handle) {
        throw flusspferd::exception("CURL handle not valid!");
      }
      return handle;
    }
    EasyOpt &get_opt() {
      return opt;
    }

    Easy(flusspferd::object const &self, flusspferd::call_context&)
      : base_type(self), handle(curl_easy_init()), opt(EasyOpt::create(*this))
      {
        if(!handle) {
          throw flusspferd::exception("curl_easy_init");
        }
      }

    Easy(flusspferd::object const &self, CURL *hnd)
      : base_type(self), handle(hnd), opt(EasyOpt::create(*this))
      {
        assert(handle);
      }

    void cleanup() {
      if(handle) {
        curl_easy_cleanup(handle);
        handle = 0x0;
      }
    }
    ~Easy() { cleanup(); }

    void perform() {
      CURLcode res = curl_easy_perform(get());
      if(res != 0) {
        throw flusspferd::exception(std::string("curl_easy_perform: ") +
                                    curl_easy_strerror(res));
      }
    }

    void reset() {
      curl_easy_reset(get());
      // TODO clear EasyOpt
    }

    std::string unescape(char const *input) {
      int len;
      char *const uesc = curl_easy_unescape(get(), input, 0, &len);
      if(!uesc) {
        throw flusspferd::exception("curl_easy_unescape");
      }
      std::string ret(uesc, len);
      curl_free(uesc);
      return ret;
    }

    std::string escape(char const *input) {
      char *const esc = curl_easy_escape(get(), input, 0);
      if(!esc) {
        throw flusspferd::exception("curl_easy_escape");
      }
      std::string ret(esc);
      curl_free(esc);
      return ret;
    }

    static Easy &create(CURL *hnd) {
      return flusspferd::create<Easy>(bf::make_vector(hnd));
    }

    //private:
    template<typename T>
      void do_setopt(CURLoption what, T data) {
      CURLcode res = curl_easy_setopt(get(), what, data);
      if(res != 0) {
        throw flusspferd::exception(std::string("curl_easy_setopt: ") +
                                    curl_easy_strerror(res));
      }
    }
  };
  Easy &wrap(CURL *hnd) {
    return Easy::create(hnd);
  }
  CURL *unwrap(Easy &c) {
    return c.data();
  }

  namespace {
    template<CURLoption What>
    struct integer_option : handle_option {
      function getter() const {
        return create<flusspferd::method>("$get_", &get);
      }
      function setter() const {
        return create<flusspferd::method>("$set_", &set);
      }
      boost::any data() const { return 0l; }
      CURLoption what() const { return What; }
    private:
      static long get(EasyOpt *o) {
        assert(o);
        return boost::any_cast<long>(o->data[What]);
      }
      static void set(EasyOpt *o, long opt) {
        assert(o);
        o->data[What] = opt;
        o->parent.do_setopt(What, opt);
      }
    };

    template<CURLoption What>
    struct string_option : handle_option {
      function getter() const {
        return create<flusspferd::method>("$get_", &get);
      }
      function setter() const {
        return create<flusspferd::method>("$set_", &set);
      }
      boost::any data() const { return std::string(); }
      CURLoption what() const { return What; }
    private:
      static std::string get(EasyOpt *o) {
        assert(o);
        return boost::any_cast<std::string>(o->data[What]);
      }
      static void set(EasyOpt *o, std::string const &val) {
        assert(o);
        o->data[What] = val;
        o->parent.do_setopt(What, boost::any_cast<std::string&>(o->data[What]).c_str());
      }
    };

    /*
      add a specialisation of this template to map to the real callback.

      this is actually a hack. If you know a better way please replace it. But it's
      better to add this for each callback than writing the complete function_option.
    */
    template<CURLoption What>
    struct map_to_callback;
    template<>
    struct map_to_callback<CURLOPT_WRITEFUNCTION> {
      typedef std::size_t (*type)(void *ptr, size_t size, size_t nmemb, void *stream);
      static type get() { return &Easy::writefunction; }
    };
    template<>
    struct map_to_callback<CURLOPT_READFUNCTION> {
      typedef std::size_t (*type)(void *ptr, size_t size, size_t nmemb, void *stream);
      static type get() { return &Easy::readfunction; }
    };
    template<>
    struct map_to_callback<CURLOPT_PROGRESSFUNCTION> {
      typedef curl_progress_callback type;
      static type get() { return &Easy::progressfunction; }
    };
    template<>
    struct map_to_callback<CURLOPT_HEADERFUNCTION> {
      typedef size_t (*type)(void *ptr, size_t size, size_t nmemb, void *stream);
      static type get() { return &Easy::headerfunction; }
    };
    template<>
    struct map_to_callback<CURLOPT_DEBUGFUNCTION> {
      typedef curl_debug_callback type;
      static type get() { return &Easy::debugfunction; }
    };

    template<CURLoption What, CURLoption WhatData,
             object (Easy::*Obj)>
    struct function_option : handle_option {
      function getter() const {
        return create<flusspferd::method>("$get_", &get);
      }
      function setter() const {
        return create<flusspferd::method>("$set_", &set);
      }
      boost::any data() const { return function(); }
      CURLoption what() const { return What; }
    private:
      static object get(EasyOpt *o) {
        assert(o);
        return o->parent.*(Obj);
      }
      static void set(EasyOpt *o, object val) {
        assert(o);
        o->parent.*(Obj) = val;
        if(val.is_null()) {
          o->parent.do_setopt(What, 0x0);
        }
        else {
          // specialise map_to_callback<What> if you want to add a new callback!
          o->parent.do_setopt(What, map_to_callback<What>::get());
          o->parent.do_setopt(WhatData, &o->parent);
        }
      }
    };

    options_map_t const &get_options() {
      /* elisp helper (+ keyboard macros):
(defun insopt (type name)
  (interactive "sTyp: \nsOpt: \n")
  (if (string= (downcase type) "i")
      (setq type "integer")
      (if (string= (downcase type) "s")
          (setq type "string")))
  (setq name (replace-regexp-in-string "^CURLOPT_" "" name))
  (insert (concat "ptr_map_insert< " type "_option<CURLOPT_" name "> >(map)(\"" (downcase name) "\");")))
(defun insreg (begin end type)
  (interactive "r\nsTyp: ")
  (insopt type (buffer-substring begin end))
  (kill-region begin end)
  (c-indent-line-or-region))
      */
      static options_map_t map;
      if(map.empty()) {
        using namespace boost::assign;
        // BEGIN DOC{
        /* HEADER{
           cURL.Easy.options -> EasyOpt

           The options property is used to set connection options (see curl_easy_setopt).

           ## Example #
           %      c.options.url = 'http://www.google.com'

           ## Options #
           The following options are supported.
           }HEADER
         */
        // BEHAVIOR OPTIONS
        ptr_map_insert< integer_option<CURLOPT_VERBOSE> >(map)("verbose");
        ptr_map_insert< integer_option<CURLOPT_HEADER> >(map)("header");
        ptr_map_insert< integer_option<CURLOPT_NOPROGRESS> >(map)("noprogress");
        ptr_map_insert< integer_option<CURLOPT_NOSIGNAL> >(map)("nosignal");
        // CALLBACK OPTIONS
        ptr_map_insert< function_option<CURLOPT_WRITEFUNCTION,
          CURLOPT_WRITEDATA, &Easy::writefunction_callback> >(map)
          ("writefunction");
        ptr_map_insert< function_option<CURLOPT_READFUNCTION,
          CURLOPT_READDATA, &Easy::readfunction_callback> >(map)
          ("readfunction");
        ptr_map_insert< function_option<CURLOPT_PROGRESSFUNCTION,
          CURLOPT_PROGRESSDATA, &Easy::progressfunction_callback> >(map)
          ("progressfunction");
        ptr_map_insert< function_option<CURLOPT_HEADERFUNCTION,
          CURLOPT_HEADERDATA, &Easy::headerfunction_callback> >(map)
          ("headerfunction");
        ptr_map_insert< function_option<CURLOPT_DEBUGFUNCTION,
          CURLOPT_DEBUGDATA, &Easy::debugfunction_callback> >(map)
          ("debugfunction");
        // ERROR OPTIONS
        ptr_map_insert< integer_option<CURLOPT_FAILONERROR> >(map)("failonerror");
        // NETWORK OPTIONS
        ptr_map_insert< integer_option<CURLOPT_PROTOCOLS> >(map)("protocols");
        ptr_map_insert< integer_option<CURLOPT_REDIR_PROTOCOLS> >(map)("redirProtocols");
        ptr_map_insert< string_option<CURLOPT_URL> >(map)("url");
        ptr_map_insert< string_option<CURLOPT_PROXY> >(map)("proxy");
        ptr_map_insert< integer_option<CURLOPT_PROXYPORT> >(map)("proxyport");
        ptr_map_insert< integer_option<CURLOPT_PROXYTYPE> >(map)("proxytype"); // See cURL.PROXY_
        ptr_map_insert< string_option<CURLOPT_NOPROXY> >(map)("noproxy");
        ptr_map_insert< integer_option<CURLOPT_HTTPPROXYTUNNEL> >(map)("httpProxyTunnel");
        ptr_map_insert< string_option<CURLOPT_SOCKS5_GSSAPI_SERVICE> >(map)("socks5GssapiService");
        ptr_map_insert< integer_option<CURLOPT_SOCKS5_GSSAPI_NEC> >(map)("socks5GssapiNec");
        ptr_map_insert< string_option<CURLOPT_INTERFACE> >(map)("interface");
        ptr_map_insert< integer_option<CURLOPT_LOCALPORT> >(map)("localport");
        ptr_map_insert< integer_option<CURLOPT_LOCALPORTRANGE> >(map)("localportrange");
        ptr_map_insert< integer_option<CURLOPT_DNS_CACHE_TIMEOUT> >(map)("dnsCacheTimeout");
        ptr_map_insert< integer_option<CURLOPT_DNS_USE_GLOBAL_CACHE> >(map)("dnsUseGlobalCache");
        ptr_map_insert< integer_option<CURLOPT_BUFFERSIZE> >(map)("buffersize");
        ptr_map_insert< integer_option<CURLOPT_PORT> >(map)("port");
        ptr_map_insert< integer_option<CURLOPT_TCP_NODELAY> >(map)("tcpNoDelay");
        ptr_map_insert< integer_option<CURLOPT_ADDRESS_SCOPE> >(map)("addressScope");
        // NAMES and PASSWORDS OPTIONS (Authentication)
        ptr_map_insert< integer_option<CURLOPT_NETRC> >(map)("netrc");
        ptr_map_insert< string_option<CURLOPT_NETRC_FILE> >(map)("netrcFile");
        ptr_map_insert< string_option<CURLOPT_USERPWD> >(map)("userpwd");
        ptr_map_insert< string_option<CURLOPT_PROXYUSERPWD> >(map)("proxyuserpwd");
        ptr_map_insert< string_option<CURLOPT_USERNAME> >(map)("username");
        ptr_map_insert< string_option<CURLOPT_PASSWORD> >(map)("password");
        ptr_map_insert< string_option<CURLOPT_PROXYUSERNAME> >(map)("proxyusername");
        ptr_map_insert< string_option<CURLOPT_PROXYPASSWORD> >(map)("proxypassword");
        ptr_map_insert< integer_option<CURLOPT_HTTPAUTH> >(map)("httpauth"); // See cURL.AUTH_*
        ptr_map_insert< integer_option<CURLOPT_PROXYAUTH> >(map)("proxyauth");
        // HTTP OPTIONS
        ptr_map_insert< integer_option<CURLOPT_AUTOREFERER> >(map)("autoreferer");
        ptr_map_insert< string_option<CURLOPT_ENCODING> >(map)("encoding");
        ptr_map_insert< integer_option<CURLOPT_FOLLOWLOCATION> >(map)("followlocation");
        ptr_map_insert< integer_option<CURLOPT_UNRESTRICTED_AUTH> >(map)("unrestrictedAuth");
        ptr_map_insert< integer_option<CURLOPT_MAXREDIRS> >(map)("maxredirs");
        ptr_map_insert< integer_option<CURLOPT_POSTREDIR> >(map)("postredir"); // See cURL.REDIR_*
        // TODO: POST*
        ptr_map_insert< string_option<CURLOPT_REFERER> >(map)("referer");
        ptr_map_insert< string_option<CURLOPT_USERAGENT> >(map)("userAgent");
        // TODO: HTTPHEADER,HTTP200ALIASES
        ptr_map_insert< string_option<CURLOPT_COOKIE> >(map)("cookie");
        ptr_map_insert< string_option<CURLOPT_COOKIEFILE> >(map)("cookieFile");
        ptr_map_insert< string_option<CURLOPT_COOKIEJAR> >(map)("cookiejar");
        ptr_map_insert< integer_option<CURLOPT_COOKIESESSION> >(map)("cookiesession");
        ptr_map_insert< string_option<CURLOPT_COOKIELIST> >(map)("cookielist");
        ptr_map_insert< integer_option<CURLOPT_HTTPGET> >(map)("httpGet");
        ptr_map_insert< integer_option<CURLOPT_HTTP_VERSION> >(map)("httpVersion"); // See cURL.HTTP_VERSION_*
        ptr_map_insert< integer_option<CURLOPT_IGNORE_CONTENT_LENGTH> >(map)("ignoreContentLength");
        ptr_map_insert< integer_option<CURLOPT_HTTP_CONTENT_DECODING> >(map)("httpContentDecoding");
        ptr_map_insert< integer_option<CURLOPT_HTTP_TRANSFER_DECODING> >(map)("httpTransferDecoding");
        // TFTP OPTIONS
        ptr_map_insert< integer_option<CURLOPT_TFTP_BLKSIZE> >(map)("tftpBlksize");

        // FTP OPTIONS
        ptr_map_insert< string_option<CURLOPT_FTPPORT> >(map)("ftpPort");
        // TODO: CURLOPT_QUOTE, CURLOPT_POSTQUOTE, CURLOPT_PREQUOTE
        ptr_map_insert< integer_option<CURLOPT_DIRLISTONLY> >(map)("dirListOnly");
        ptr_map_insert< integer_option<CURLOPT_APPEND> >(map)("append");
        ptr_map_insert< integer_option<CURLOPT_FTP_USE_EPRT> >(map)("ftpUseEprt");
        ptr_map_insert< integer_option<CURLOPT_FTP_USE_EPSV> >(map)("ftpUseEpsv");
        ptr_map_insert< integer_option<CURLOPT_FTP_CREATE_MISSING_DIRS> >(map)("ftpCreateMissingDirs");
        ptr_map_insert< integer_option<CURLOPT_FTP_RESPONSE_TIMEOUT> >(map)("ftpResponseTimeout");
        ptr_map_insert< string_option<CURLOPT_FTP_ALTERNATIVE_TO_USER> >(map)("ftpAltnerativeToUser");
        ptr_map_insert< integer_option<CURLOPT_FTP_SKIP_PASV_IP> >(map)("ftpSkipPasvIp");
        ptr_map_insert< integer_option<CURLOPT_USE_SSL> >(map)("usessl");
        ptr_map_insert< integer_option<CURLOPT_FTPSSLAUTH> >(map)("ftpsslauth");
        ptr_map_insert< integer_option<CURLOPT_FTP_SSL_CCC> >(map)("ftpSslCcc");
        ptr_map_insert< string_option<CURLOPT_FTP_ACCOUNT> >(map)("ftpAccount");
        ptr_map_insert< integer_option<CURLOPT_FTP_FILEMETHOD> >(map)("ftpFilemethod");
        // PROTOCOL OPTIONS
        ptr_map_insert< integer_option<CURLOPT_TRANSFERTEXT> >(map)("transfertext");
        ptr_map_insert< integer_option<CURLOPT_PROXY_TRANSFER_MODE> >(map)("proxyTransferMode");
        ptr_map_insert< integer_option<CURLOPT_CRLF> >(map)("crlf");
        ptr_map_insert< string_option<CURLOPT_RANGE> >(map)("range");
        ptr_map_insert< integer_option<CURLOPT_RESUME_FROM> >(map)("resumeFrom");
        ptr_map_insert< string_option<CURLOPT_CUSTOMREQUEST> >(map)("customrequest");
        ptr_map_insert< integer_option<CURLOPT_FILETIME> >(map)("filetime");
        ptr_map_insert< integer_option<CURLOPT_NOBODY> >(map)("nobody");
        ptr_map_insert< integer_option<CURLOPT_INFILESIZE> >(map)("infilesize");
        ptr_map_insert< integer_option<CURLOPT_UPLOAD> >(map)("upload");
        ptr_map_insert< integer_option<CURLOPT_MAXFILESIZE> >(map)("maxfilesize");
        ptr_map_insert< integer_option<CURLOPT_TIMECONDITION> >(map)("timecondition");
        ptr_map_insert< integer_option<CURLOPT_TIMEVALUE> >(map)("timevalue");
        // CONNECTION OPTIONS
        ptr_map_insert< integer_option<CURLOPT_TIMEOUT> >(map)("timeout");
        ptr_map_insert< integer_option<CURLOPT_TIMEOUT_MS> >(map)("timeoutMS");
        ptr_map_insert< integer_option<CURLOPT_LOW_SPEED_LIMIT> >(map)("lowSpeedLimit");
        ptr_map_insert< integer_option<CURLOPT_LOW_SPEED_TIME> >(map)("lowSpeedTime");
        ptr_map_insert< integer_option<CURLOPT_MAXCONNECTS> >(map)("maxconnects");
        ptr_map_insert< integer_option<CURLOPT_FRESH_CONNECT> >(map)("freshConnect");
        ptr_map_insert< integer_option<CURLOPT_FORBID_REUSE> >(map)("forbidReuse");
        ptr_map_insert< integer_option<CURLOPT_CONNECTTIMEOUT> >(map)("connecttimeout");
        ptr_map_insert< integer_option<CURLOPT_CONNECTTIMEOUT_MS> >(map)("connecttimeoutMS");
        ptr_map_insert< integer_option<CURLOPT_IPRESOLVE> >(map)("ipresolve"); // See cURL.IPRESOLVE_*
        ptr_map_insert< integer_option<CURLOPT_CONNECT_ONLY> >(map)("connectOnly");
        // SSL and SECURITY OPTIONS
        ptr_map_insert< string_option<CURLOPT_SSLCERT> >(map)("sslcert");
        ptr_map_insert< string_option<CURLOPT_SSLCERTTYPE> >(map)("sslcerttype");
        ptr_map_insert< string_option<CURLOPT_SSLKEY> >(map)("sslkey");
        ptr_map_insert< string_option<CURLOPT_SSLKEYTYPE> >(map)("sslkeytype");
        ptr_map_insert< string_option<CURLOPT_KEYPASSWD> >(map)("keypasswd");
        ptr_map_insert< string_option<CURLOPT_SSLENGINE> >(map)("sslengine");
        ptr_map_insert< string_option<CURLOPT_SSLENGINE_DEFAULT> >(map)("sslengineDefault");
        ptr_map_insert< integer_option<CURLOPT_SSLVERSION> >(map)("sslversion"); // See cURL.SSLVERSION_*
        ptr_map_insert< integer_option<CURLOPT_SSL_VERIFYPEER> >(map)("sslVerifypeer");
        ptr_map_insert< string_option<CURLOPT_CAINFO> >(map)("cainfo");
        ptr_map_insert< string_option<CURLOPT_ISSUERCERT> >(map)("issuercert");
        ptr_map_insert< string_option<CURLOPT_CAPATH> >(map)("capath");
        ptr_map_insert< string_option<CURLOPT_CRLFILE> >(map)("crlfile");
        ptr_map_insert< integer_option<CURLOPT_CERTINFO> >(map)("certinfo");
        ptr_map_insert< string_option<CURLOPT_RANDOM_FILE> >(map)("randomFile");
        ptr_map_insert< string_option<CURLOPT_EGDSOCKET> >(map)("egdsocket");
        ptr_map_insert< integer_option<CURLOPT_SSL_VERIFYHOST> >(map)("sslVerifyhost");
        ptr_map_insert< string_option<CURLOPT_SSL_CIPHER_LIST> >(map)("sslCipherList");
        ptr_map_insert< integer_option<CURLOPT_SSL_SESSIONID_CACHE> >(map)("sslSessionidCache");
        ptr_map_insert< string_option<CURLOPT_KRBLEVEL> >(map)("krblevel");
        // SSH OPTIONS
        ptr_map_insert< integer_option<CURLOPT_SSH_AUTH_TYPES> >(map)("sshAuthTypes"); // See cURL.SSH_AUTH*
        ptr_map_insert< string_option<CURLOPT_SSH_HOST_PUBLIC_KEY_MD5> >(map)("sshHostPublicKeyMd5");
        ptr_map_insert< string_option<CURLOPT_SSH_PUBLIC_KEYFILE> >(map)("sshPublicKeyfile");
        ptr_map_insert< string_option<CURLOPT_SSH_PRIVATE_KEYFILE> >(map)("sshPrivateKeyfile");
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 6)
        ptr_map_insert< string_option<CURLOPT_SSH_KNOWNHOSTS> >(map)("sshKnownhosts");
#endif
        // TODO: SSH_KEYFUNCTION/DATA
        // OTHER OPTIONS
        ptr_map_insert< integer_option<CURLOPT_NEW_FILE_PERMS> >(map)("newFilePerms");
        ptr_map_insert< integer_option<CURLOPT_NEW_DIRECTORY_PERMS> >(map)("newDirectoryPerms");
        // }END DOC
      }
      return map;
    }
  }

  bool EasyOpt::property_resolve(value const &id, unsigned) {
    std::string const name = id.to_std_string();
    options_map_t::const_iterator const i = get_options().find(name);
    if(i != get_options().end()) {
      property_attributes attr(no_property_flag,
                               i->second->getter(),
                               i->second->setter());
      define_property(id.get_string(), value(), attr);
      data[i->second->what()] = i->second->data();
      return true;
    }
    else {
      return false;
    }
  }

  FLUSSPFERD_LOADER_SIMPLE(cURL) {
    local_root_scope scope;

    create<flusspferd::function>("globalInit", &global_init, param::_container = cURL);
    load_class<EasyOpt>(cURL);
    load_class<Easy>(cURL);

    cURL.define_properties(read_only_property | permanent_property)
        ("version", value(curl_version()))
        ("GLOBAL_ALL", value(CURL_GLOBAL_ALL))
        ("GLOBAL_SSL", value(CURL_GLOBAL_SSL))
        ("GLOBAL_WIN32", value(CURL_GLOBAL_WIN32))
        ("GLOBAL_NOTHING", value(CURL_GLOBAL_NOTHING))
        ("PROTO_HTTP", value(static_cast<int>(CURLPROTO_HTTP)))
        ("PROTO_HTTPS", value(static_cast<int>(CURLPROTO_HTTPS)))
        ("PROTO_FTP", value(static_cast<int>(CURLPROTO_FTP)))
        ("PROTO_FTPS", value(static_cast<int>(CURLPROTO_FTPS)))
        ("PROTO_SCP", value(static_cast<int>(CURLPROTO_SCP)))
        ("PROTO_SFTP", value(static_cast<int>(CURLPROTO_SFTP)))
        ("PROTO_TELNET", value(static_cast<int>(CURLPROTO_TELNET)))
        ("PROTO_LDAP", value(static_cast<int>(CURLPROTO_LDAP)))
        ("PROTO_LDAPS", value(static_cast<int>(CURLPROTO_LDAPS)))
        ("PROTO_DICT", value(static_cast<int>(CURLPROTO_DICT)))
        ("PROTO_FILE", value(static_cast<int>(CURLPROTO_FILE)))
        ("PROTO_TFTP", value(static_cast<int>(CURLPROTO_TFTP)))
        ("PROTO_ALL", value(static_cast<int>(CURLPROTO_ALL)))
        ("INFO_TEXT", value(static_cast<int>(CURLINFO_TEXT)))
        ("INFO_HEADER_IN", value(static_cast<int>(CURLINFO_HEADER_IN)))
        ("INFO_HEADER_OUT", value(static_cast<int>(CURLINFO_HEADER_OUT)))
        ("INFO_DATA_IN", value(static_cast<int>(CURLINFO_DATA_IN)))
        ("INFO_DATA_OUT", value(static_cast<int>(CURLINFO_DATA_OUT)))
        ("PROXY_HTTP", value(static_cast<int>(CURLPROXY_HTTP)))
        ("PROXY_HTTP_1_0", value(static_cast<int>(CURLPROXY_HTTP_1_0)))
        ("PROXY_SOCKS4", value(static_cast<int>(CURLPROXY_SOCKS4)))
        ("PROXY_SOCKS5", value(static_cast<int>(CURLPROXY_SOCKS5)))
        ("PROXY_SOCKS4A", value(static_cast<int>(CURLPROXY_SOCKS4A)))
        ("PROXY_SOCKS5_HOSTNAME", value(static_cast<int>(CURLPROXY_SOCKS5_HOSTNAME)))
        ("NETRC_OPTIONAL", value(static_cast<int>(CURL_NETRC_OPTIONAL)))
        ("NETRC_IGNORED", value(static_cast<int>(CURL_NETRC_IGNORED)))
        ("NETRC_REQUIRED", value(static_cast<int>(CURL_NETRC_REQUIRED)))
        ("AUTH_BASIC", value(static_cast<int>(CURLAUTH_BASIC)))
        ("AUTH_DIGEST", value(static_cast<int>(CURLAUTH_DIGEST)))
        ("AUTH_DIGEST_IE", value(static_cast<int>(CURLAUTH_DIGEST_IE)))
        ("AUTH_GSSNEGOTIATE", value(static_cast<int>(CURLAUTH_GSSNEGOTIATE)))
        ("AUTH_NTLM", value(static_cast<int>(CURLAUTH_NTLM)))
        ("AUTH_ANY", value(static_cast<int>(CURLAUTH_ANY)))
        ("AUTH_ANYSAFE", value(static_cast<int>(CURLAUTH_ANYSAFE)))
        ("REDIR_POST_301", value(static_cast<int>(CURL_REDIR_POST_301)))
        ("REDIR_POST_302", value(static_cast<int>(CURL_REDIR_POST_302)))
        ("REDIR_POST_ALL", value(static_cast<int>(CURL_REDIR_POST_ALL)))
        ("HTTP_VERSION_NONE", value(static_cast<int>(CURL_HTTP_VERSION_NONE)))
        ("HTTP_VERSION_1_0", value(static_cast<int>(CURL_HTTP_VERSION_1_0)))
        ("HTTP_VERSION_1_1", value(static_cast<int>(CURL_HTTP_VERSION_1_1)))
        ("USESSL_NONE", value(static_cast<int>(CURLUSESSL_NONE)))
        ("USESSL_TRY", value(static_cast<int>(CURLUSESSL_TRY)))
        ("USESSL_CONTROL", value(static_cast<int>(CURLUSESSL_CONTROL)))
        ("USESSL_ALL", value(static_cast<int>(CURLUSESSL_ALL)))
        ("FTPAUTH_DEFAULT", value(static_cast<int>(CURLFTPAUTH_DEFAULT)))
        ("FTPAUTH_SSL", value(static_cast<int>(CURLFTPAUTH_SSL)))
        ("FTPAUTH_TLS", value(static_cast<int>(CURLFTPAUTH_TLS)))
        ("FTPSSL_CCC_NONE", value(static_cast<int>(CURLFTPSSL_CCC_NONE)))
        ("FTPSSL_CCC_PASSIVE", value(static_cast<int>(CURLFTPSSL_CCC_PASSIVE)))
        ("FTPSSL_CCC_ACTIVE", value(static_cast<int>(CURLFTPSSL_CCC_ACTIVE)))
        ("FTPMETHOD_MULTICWD", value(static_cast<int>(CURLFTPMETHOD_MULTICWD)))
        ("FTPMETHOD_NOCWD", value(static_cast<int>(CURLFTPMETHOD_NOCWD)))
        ("FTPMETHOD_SINGLECWD", value(static_cast<int>(CURLFTPMETHOD_SINGLECWD)))
        ("IPRESOLVE_WHATEVER", value(static_cast<int>(CURL_IPRESOLVE_WHATEVER)))
        ("IPRESOLVE_V4", value(static_cast<int>(CURL_IPRESOLVE_V4)))
        ("IPRESOLVE_V6", value(static_cast<int>(CURL_IPRESOLVE_V6)))
        ("SSLVERSION_DEFAULT", value(static_cast<int>(CURL_SSLVERSION_DEFAULT)))
        ("SSLVERSION_TLSv1", value(static_cast<int>(CURL_SSLVERSION_TLSv1)))
        ("SSLVERSION_SSLv2", value(static_cast<int>(CURL_SSLVERSION_SSLv2)))
        ("SSLVERSION_SSLv3", value(static_cast<int>(CURL_SSLVERSION_SSLv3)));

  }
}
