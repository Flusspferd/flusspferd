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

//#include <iostream> //DEBUG

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
			static options_map_t map;
			if(map.empty()) {
				using namespace boost::assign;
        // BEHAVIOR OPTIONS
				ptr_map_insert< integer_option<CURLOPT_VERBOSE> >(map)
					("verbose");
				ptr_map_insert< integer_option<CURLOPT_HEADER> >(map)
					("header");
				ptr_map_insert< integer_option<CURLOPT_NOPROGRESS> >(map)
					("noProgress");
				ptr_map_insert< integer_option<CURLOPT_NOSIGNAL> >(map)
					("noSignal");
        // CALLBACK OPTIONS
				ptr_map_insert< function_option<CURLOPT_WRITEFUNCTION,
					CURLOPT_WRITEDATA, &Easy::writefunction_callback> >(map)
					("writeFunction");
				ptr_map_insert< function_option<CURLOPT_READFUNCTION,
					CURLOPT_READDATA, &Easy::readfunction_callback> >(map)
					("readFunction");
				ptr_map_insert< function_option<CURLOPT_PROGRESSFUNCTION,
					CURLOPT_PROGRESSDATA, &Easy::progressfunction_callback> >(map)
					("progressFunction");
        ptr_map_insert< function_option<CURLOPT_DEBUGFUNCTION,
					CURLOPT_DEBUGDATA, &Easy::debugfunction_callback> >(map)
					("debugFunction");
        // ERROR OPTIONS
        ptr_map_insert< integer_option<CURLOPT_FAILONERROR> >(map)("failOnError");
        // NETWORK OPTIONS
        ptr_map_insert< string_option<CURLOPT_URL> >(map)("url");
        ptr_map_insert< string_option<CURLOPT_PROXY> >(map)("proxy");
        ptr_map_insert< integer_option<CURLOPT_PROXYPORT> >(map)("proxyPort");
        ptr_map_insert< integer_option<CURLOPT_PROXYTYPE> >(map)("proxyType"); // see cURL.PROXY_
        ptr_map_insert< string_option<CURLOPT_NOPROXY> >(map)("noProxy");
        ptr_map_insert< integer_option<CURLOPT_HTTPPROXYTUNNEL> >(map)("httpProxyTunnel");
        ptr_map_insert< string_option<CURLOPT_SOCKS5_GSSAPI_SERVICE> >(map)("socks5GssapiService");
        ptr_map_insert< integer_option<CURLOPT_SOCKS5_GSSAPI_NEC> >(map)("socks5GssapiNec");
				ptr_map_insert< string_option<CURLOPT_INTERFACE> >(map)("interface");
        ptr_map_insert< integer_option<CURLOPT_LOCALPORT> >(map)("localPort");
        ptr_map_insert< integer_option<CURLOPT_LOCALPORTRANGE> >(map)("localPortRange");
        ptr_map_insert< integer_option<CURLOPT_DNS_CACHE_TIMEOUT> >(map)("dnsCacheTimeout");
        ptr_map_insert< integer_option<CURLOPT_DNS_USE_GLOBAL_CACHE> >(map)("dnsUseGlobalCache");
				ptr_map_insert< integer_option<CURLOPT_PORT> >(map)("port");
        ptr_map_insert< integer_option<CURLOPT_TCP_NODELAY> >(map)("tcpNoDelay");
        ptr_map_insert< integer_option<CURLOPT_ADDRESS_SCOPE> >(map)("addressScope");
        // NAMES and PASSWORDS OPTIONS (Authentication)
        ptr_map_insert< string_option<CURLOPT_USERPWD> >(map)("userPwd");
        ptr_map_insert< string_option<CURLOPT_PROXYUSERPWD> >(map)("proxyUserPwd");
        ptr_map_insert< string_option<CURLOPT_USERNAME> >(map)("username");
        ptr_map_insert< string_option<CURLOPT_PASSWORD> >(map)("password");
        ptr_map_insert< string_option<CURLOPT_PROXYUSERNAME> >(map)("proxyUsername");
        ptr_map_insert< string_option<CURLOPT_PROXYPASSWORD> >(map)("proxyPassword");
        ptr_map_insert< integer_option<CURLOPT_HTTPAUTH> >(map)("httpAuth"); // see cURL.AUTH_*
        ptr_map_insert< integer_option<CURLOPT_PROXYAUTH> >(map)("proxyAuth");
        // HTTP OPTIONS
        ptr_map_insert< integer_option<CURLOPT_AUTOREFERER> >(map)("autoReferer");
        ptr_map_insert< string_option<CURLOPT_ENCODING> >(map)("encoding");
        ptr_map_insert< integer_option<CURLOPT_FOLLOWLOCATION> >(map)("followLocation");
        ptr_map_insert< integer_option<CURLOPT_UNRESTRICTED_AUTH> >(map)("unrestrictedAuth");
        ptr_map_insert< integer_option<CURLOPT_MAXREDIRS> >(map)("maxRedirs");
        ptr_map_insert< integer_option<CURLOPT_POSTREDIR> >(map)("postRedir"); // see cURL.REDIR_*
        // TODO POST*
        ptr_map_insert< string_option<CURLOPT_REFERER> >(map)("referer");
				ptr_map_insert< string_option<CURLOPT_USERAGENT> >(map)("userAgent");
        // TODO HTTPHEADER,HTTP200ALIASES
				ptr_map_insert< string_option<CURLOPT_COOKIE> >(map)("cookie");
        ptr_map_insert< string_option<CURLOPT_COOKIEFILE> >(map)("cookieFile");
        ptr_map_insert< string_option<CURLOPT_COOKIEJAR> >(map)("cookieJar");
        ptr_map_insert< integer_option<CURLOPT_COOKIESESSION> >(map)("cookieSession");
        ptr_map_insert< string_option<CURLOPT_COOKIELIST> >(map)("cookieList");
        ptr_map_insert< integer_option<CURLOPT_HTTPGET> >(map)("httpGet");
        ptr_map_insert< integer_option<CURLOPT_HTTP_VERSION> >(map)("httpVersion"); // see cURL.HTTP_VERSION_*
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
        // SSH OPTIONS
        ptr_map_insert< integer_option<CURLOPT_SSH_AUTH_TYPES> >(map)("sshAuthTypes"); // see cURL.SSH_AUTH*
        ptr_map_insert< string_option<CURLOPT_SSH_HOST_PUBLIC_KEY_MD5> >(map)("sshHostPublicKeyMD5");
        ptr_map_insert< string_option<CURLOPT_SSH_PUBLIC_KEYFILE> >(map)("sshPublicKeyfile");
        ptr_map_insert< string_option<CURLOPT_SSH_PRIVATE_KEYFILE> >(map)("sshPrivateKeyfile");
#if (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19 && LIBCURL_VERSION_PATH >= 6)
        ptr_map_insert< string_option<CURLOPT_SSH_KNOWNHOSTS> >(map)("sshKnownHosts");
#endif
        // TODO: SSH_KEYFUNCTION/DATA
        // OTHER OPTIONS
        ptr_map_insert< integer_option<CURLOPT_NEW_FILE_PERMS> >(map)("newFilePerms");
        ptr_map_insert< integer_option<CURLOPT_NEW_DIRECTORY_PERMS> >(map)("newDirectoryPerms");
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

    cURL.define_property("GLOBAL_ALL", value(CURL_GLOBAL_ALL),
                         read_only_property | permanent_property);
    cURL.define_property("GLOBAL_SSL", value(CURL_GLOBAL_SSL),
                         read_only_property | permanent_property);
    cURL.define_property("GLOBAL_WIN32", value(CURL_GLOBAL_WIN32),
                         read_only_property | permanent_property);
    cURL.define_property("GLOBAL_NOTHING", value(CURL_GLOBAL_NOTHING),
                         read_only_property | permanent_property);
    create<flusspferd::function>("globalInit", &global_init, param::_container = cURL);
    cURL.define_property("version", value(curl_version()),
                         read_only_property | permanent_property);
		load_class<EasyOpt>(cURL);
    load_class<Easy>(cURL);
    cURL.define_property("INFO_TEXT", value(static_cast<int>(CURLINFO_TEXT)),
                         read_only_property | permanent_property);
    cURL.define_property("INFO_HEADER_IN", value(static_cast<int>(CURLINFO_HEADER_IN)),
                         read_only_property | permanent_property);
    cURL.define_property("INFO_HEADER_OUT", value(static_cast<int>(CURLINFO_HEADER_OUT)),
                         read_only_property | permanent_property);
    cURL.define_property("INFO_DATA_IN", value(static_cast<int>(CURLINFO_DATA_IN)),
                         read_only_property | permanent_property);
    cURL.define_property("INFO_DATA_OUT", value(static_cast<int>(CURLINFO_DATA_OUT)),
                         read_only_property | permanent_property);
    cURL.define_property("PROXY_HTTP", value(static_cast<int>(CURLPROXY_HTTP)),
                         read_only_property | permanent_property);
    cURL.define_property("PROXY_HTTP_1_0", value(static_cast<int>(CURLPROXY_HTTP_1_0)),
                         read_only_property | permanent_property);
    cURL.define_property("PROXY_SOCKS4", value(static_cast<int>(CURLPROXY_SOCKS4)),
                         read_only_property | permanent_property);
    cURL.define_property("PROXY_SOCKS5", value(static_cast<int>(CURLPROXY_SOCKS5)),
                         read_only_property | permanent_property);
    cURL.define_property("PROXY_SOCKS4A", value(static_cast<int>(CURLPROXY_SOCKS4A)),
                         read_only_property | permanent_property);
    cURL.define_property("PROXY_SOCKS5_HOSTNAME", 
                         value(static_cast<int>(CURLPROXY_SOCKS5_HOSTNAME)),
                         read_only_property | permanent_property);
    cURL.define_property("AUTH_BASIC", value(static_cast<int>(CURLAUTH_BASIC)),
                         read_only_property | permanent_property);
    cURL.define_property("AUTH_DIGEST", value(static_cast<int>(CURLAUTH_DIGEST)),
                         read_only_property | permanent_property);
    cURL.define_property("AUTH_DIGEST_IE", value(static_cast<int>(CURLAUTH_DIGEST_IE)),
                         read_only_property | permanent_property);
    cURL.define_property("AUTH_GSSNEGOTIATE", value(static_cast<int>(CURLAUTH_GSSNEGOTIATE)),
                         read_only_property | permanent_property);
    cURL.define_property("AUTH_NTLM", value(static_cast<int>(CURLAUTH_NTLM)),
                         read_only_property | permanent_property);
    cURL.define_property("AUTH_ANY", value(static_cast<int>(CURLAUTH_ANY)),
                         read_only_property | permanent_property);
    cURL.define_property("AUTH_ANYSAFE", value(static_cast<int>(CURLAUTH_ANYSAFE)),
                         read_only_property | permanent_property);
    cURL.define_property("REDIR_POST_301", value(static_cast<int>(CURL_REDIR_POST_301)),
                         read_only_property | permanent_property);
    cURL.define_property("REDIR_POST_302", value(static_cast<int>(CURL_REDIR_POST_302)),
                         read_only_property | permanent_property);
    cURL.define_property("REDIR_POST_ALL", value(static_cast<int>(CURL_REDIR_POST_ALL)),
                         read_only_property | permanent_property);
    cURL.define_property("HTTP_VERSION_NONE", value(static_cast<int>(CURL_HTTP_VERSION_NONE)),
                         read_only_property | permanent_property);
    cURL.define_property("HTTP_VERSION_1_0", value(static_cast<int>(CURL_HTTP_VERSION_1_0)),
                         read_only_property | permanent_property);
    cURL.define_property("HTTP_VERSION_1_1", value(static_cast<int>(CURL_HTTP_VERSION_1_1)),
                         read_only_property | permanent_property);
  }
}
