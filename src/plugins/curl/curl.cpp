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

#include "flusspferd/array.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/binary.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/modules.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/class_description.hpp"
#include "flusspferd/property_iterator.hpp"

#include "flusspferd/create/array.hpp"

#include <sstream>
#include <curl/curl.h>

#include <boost/ptr_container/ptr_unordered_map.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/assign/ptr_map_inserter.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/thread/mutex.hpp>
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

  /*
   * cURL.Easy#options implementation:
   *
   * handle_option is the base class for option mappers to map between CURLOPT_*
   * and JavaScript. Data is stored as boost::any in EasyOpt::data. Getters/setters
   * are created (handle_option::getter/setter) on the fly in EasyOpt::property_resolve.
   * get_options() returns a map of all property names to their corresponding handle_option.
   */

  class Easy;

  namespace {
    struct handle_option {
      virtual ~handle_option() =0;
      virtual function getter() const =0;
      virtual function setter() const =0;
      // initial data
      virtual boost::any data() const =0;
      virtual CURLoption what() const =0;
      // called during tracing to prevent the GC from collecting data
      virtual void trace(boost::any const &, flusspferd::tracer &) const { }
      // called during destruction/cleanup of EasyOpt
      virtual void cleanup(boost::any &) const { }
    };
    handle_option::~handle_option() { }

    typedef boost::ptr_unordered_map<std::string, handle_option> options_map_t;
    options_map_t const &get_options();
  }

  // the class behind cURL.Easy#options.
  FLUSSPFERD_CLASS_DESCRIPTION
  (
   EasyOpt,
   (constructor_name, "EasyOpt")
   (full_name, "cURL.Easy.EasyOpt")
   (constructible, false)
   )
  {
  public: // TODO this should be private
    typedef boost::unordered_map<CURLoption, boost::any> data_map_t;
    data_map_t data;
    Easy &parent;
  public:
    EasyOpt(flusspferd::object const &self, Easy &parent)
      : base_type(self), parent(parent)
    {	}
    ~EasyOpt() {
      for(options_map_t::const_iterator i = get_options().begin();
          i != get_options().end();
          ++i)
      {
        data_map_t::iterator j = data.find(i->second->what());
        if(j != data.end()) {
          i->second->cleanup(j->second);
        }
      }
    }

    static EasyOpt &create(Easy &p) {
      return flusspferd::create<EasyOpt>(bf::make_vector(boost::ref(p)));
    }

    void clear() {
      for(options_map_t::const_iterator i = get_options().begin();
          i != get_options().end();
          ++i)
      {
        if(has_property(i->first)) {
          delete_property(i->first);
        }
      }
      data.clear();
    }
  protected:
    bool property_resolve(value const &id, unsigned access);

    void trace(flusspferd::tracer &trc) {
      for(data_map_t::const_iterator i = data.begin();
          i != data.end();
          ++i)
      {
        for(options_map_t::const_iterator j = get_options().begin();
            j != get_options().end();
            ++j)
        {
          if(j->second->what() == i->first) {
            j->second->trace(i->second, trc);
            break;
          }
        }
      }
    }
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

  public: // TODO this should be private
    /*
     * Callbacks are implemented by storing the javascript callback in an object
     * (named `function`_callback;) and providing a static function which gets called
     * by curl and than calls the javascript callback.
     *
     * See function_option below.
     *
     * WARNING: make sure the callback object gets traced! (see Easy::trace)
     */

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

    object seekfunction_callback;
    static int seekfunction(void *stream, curl_off_t offset, int origin) {
      assert(stream);
      Easy &self = *reinterpret_cast<Easy*>(stream);
      if(self.readfunction_callback.is_null()) {
        return CURL_SEEKFUNC_CANTSEEK;
      }
      else {
        arguments arg;
        arg.push_back(value(offset));
        arg.push_back(value(origin));
        value v = self.seekfunction_callback.call(arg);
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
      trc("seekFunction", seekfunction_callback);
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
      opt.clear();
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

    template<CURLoption What>
    struct list_option : handle_option {
      function getter() const {
        return create<flusspferd::method>("$get_", &get);
      }
      function setter() const {
        return create<flusspferd::method>("$set_", &set);
      }
      boost::any data() const { return data_t(create<array>(), 0x0); }
      CURLoption what() const { return What; }
      void trace(boost::any const &data, flusspferd::tracer &trc) const {
        data_t const &d = boost::any_cast<data_t const&>(data);
        if(d.second) {
          trc("list", d.first);
        }
      }
      void cleanup(boost::any &data) const {
        data_t &d = boost::any_cast<data_t&>(data);
        if(d.second) {
          curl_slist_free_all(d.second);
          d.second = 0x0;
        }
      }
    private:
      typedef std::pair<array,curl_slist*> data_t;
      static void reset(data_t &d) {
        if(d.second) {
          curl_slist_free_all(d.second);
          d.second = 0x0;
          d.first = create<array>();
        }
      }
      static array get(EasyOpt *o) {
        assert(o);
        return boost::any_cast<data_t&>(o->data[What]).first;
      }
      static void set(EasyOpt *o, array ain) {
        assert(o);
        data_t d(
          create<array>(boost::make_iterator_range(ain.begin(), ain.end())),
          0x0);
        try {
          for(array::iterator i = d.first.begin(); i != d.first.end(); ++i) {
            if(!i->is_string()) {
              throw flusspferd::exception("array data not a string");
            }
            curl_slist *r = curl_slist_append(
              d.second, i->get_string().c_str());
            if(!r) {
              throw flusspferd::exception("curl_slist_append");
            }
            d.second = r;
          }
        }
        catch(...) {
          reset(d);
          throw;
        }
        o->parent.do_setopt(What, d.second);
        data_t &old = boost::any_cast<data_t&>(o->data[What]);
        reset(old);
        old = d;
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
    struct map_to_callback<CURLOPT_SEEKFUNCTION> {
      typedef int (*type)(void *instream, curl_off_t offset, int origin);
      static type get() { return &Easy::seekfunction; }
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

    struct http_post_option : handle_option {
      static CURLoption const What;
      function getter() const {
        return create<flusspferd::method>("$get_httppost", &get);
      }
      function setter() const {
        return create<flusspferd::method>("$set_httppost", &set);
      }
      boost::any data() const { return create<array>(); }
      CURLoption what() const { return What; }
      void trace(boost::any const &data, flusspferd::tracer &trc) const {
        trc("httppost", boost::any_cast<array>(data));
      }
    private:
      static object get(EasyOpt *o) {
        assert(o);
        return boost::any_cast<array>(o->data[What]);
      }
      static char const *get_data_ptr(value v) {
        if(!v.is_string()) { // TODO binary stuff
          throw flusspferd::exception("data is not a string");
        }
        return v.get_string().c_str();
      }

      static void add_data(std::vector<curl_forms> &forms, CURLformoption option,
                           char const *name, object o)
      {
        if(o.has_property(name)) {
          curl_forms form;
          form.option = option;
          form.value = get_data_ptr(o.get_property(name));
          forms.push_back(form);
        }
      }
      static void add_length(std::vector<curl_forms> &forms, CURLformoption option,
                             char const *name, object o)
      {
        if(o.has_property(name)) {
          curl_forms form;
          form.option = option;
          value v = o.get_property(name);
          if(!v.is_int()) {
            throw flusspferd::exception("data is not an int");
          }
          // yay curl's api is weird
          form.value = reinterpret_cast<char const*>(v.get_int());
          forms.push_back(form);
        }
      }
      static void object2form(object o, curl_httppost *&post, curl_httppost *&last) {
        if(!o.has_property("name")) {
          throw flusspferd::exception("object has no `name' property");
        }
        std::vector<curl_forms> forms;
        curl_forms form;
        add_data(forms, CURLFORM_PTRCONTENTS, "contents", o);
        add_length(forms, CURLFORM_CONTENTSLENGTH, "contentslength", o);
        add_length(forms, CURLFORM_CONTENTSLENGTH, "contentsLength", o); // alt
        add_data(forms, CURLFORM_FILECONTENT, "filecontent", o);
        add_data(forms, CURLFORM_FILECONTENT, "fileContent", o); // alt
        add_data(forms, CURLFORM_FILE, "file", o);
        add_data(forms, CURLFORM_CONTENTTYPE, "contenttype", o);
        add_data(forms, CURLFORM_CONTENTTYPE, "contentType", o); // alt
        add_data(forms, CURLFORM_FILENAME, "filename", o);
        add_data(forms, CURLFORM_FILENAME, "fileName", o); // alt
        // TODO buffer ...
        form.option = CURLFORM_END;
        form.value = 0x0;
        forms.push_back(form);
        CURLFORMcode ret = curl_formadd(
          &post, &last,
          CURLFORM_PTRNAME, get_data_ptr(o.get_property("name")),
          CURLFORM_ARRAY, &forms[0], CURLFORM_END); // TODO free data
        if(ret != 0) {
          std::stringstream sstr;
          sstr << "curl_formadd failed! " << static_cast<unsigned>(ret);
          throw flusspferd::exception(sstr.str());
        }
      }
      static void set(EasyOpt *o, object val) {
        // TODO free old data
        assert(o);
        curl_httppost *post = 0x0;
        curl_httppost *last = 0x0;
        if(val.is_array()) {
          // copy the array
          array const ain(val);
          array a = flusspferd::create<flusspferd::array>(
            boost::make_iterator_range(ain.begin(), ain.end()));
          for(array::iterator i = a.begin(); i != a.end(); ++i) {
            if(!i->is_object()) {
              throw flusspferd::exception("array member not an object");
            }
            object2form(i->get_object(), post, last);
          }
          o->data[What] = a;
        }
        else {
          array a = flusspferd::create<flusspferd::array>();
          a.push(val);
          o->data[What] = a;
          object2form(val, post, last);
        }
        o->parent.do_setopt(What, post);
      }
    };
    CURLoption const http_post_option::What = CURLOPT_HTTPPOST;

    options_map_t const &get_options() {
      /* elisp helper (+ keyboard macros):
(defun insopt (type name)
  (interactive "sTyp: \nsOpt: \n")
  (if (string= (downcase type) "i")
      (setq type "integer")
      (if (string= (downcase type) "s")
          (setq type "string")
          (if (string= (downcase type) "l")
              (setq type "list"))))
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
        // this is used to auto generate some documentation. See gen-doc.js!
        // BEGIN DOC{
        /* HEADER{
           cURL.Easy#options -> EasyOpt

           The options property is used to set connection options (see curl_easy_setopt).

           ## Example #
           %      c.options.url = 'http://www.google.com'

           ## Options #
           The following options are supported. See the libcurl documentation links.
           Flag parameters are class constants of [[cURL]] without the leading `CURL(_)`.
           (e.g. `CURLPROTO_HTTP` -> `cURL.PROTO_HTTP`)

           + integer: boolean or number
           + string: string
           + list: array of strings
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
        ptr_map_insert< function_option<CURLOPT_SEEKFUNCTION,
          CURLOPT_SEEKDATA, &Easy::seekfunction_callback> >(map)
          ("seekfunction");
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
        ptr_map_insert< integer_option<CURLOPT_POST> >(map)("post");
        ptr_map_insert< string_option<CURLOPT_POSTFIELDS> >(map)("postfields");
        ptr_map_insert< integer_option<CURLOPT_POSTFIELDSIZE> >(map)("postfieldsize");
        ptr_map_insert< http_post_option >(map)("httppost");
        ptr_map_insert< string_option<CURLOPT_REFERER> >(map)("referer");
        ptr_map_insert< string_option<CURLOPT_USERAGENT> >(map)("userAgent");
        ptr_map_insert< list_option<CURLOPT_HTTPHEADER> >(map)("httpheader");
        ptr_map_insert< list_option<CURLOPT_HTTP200ALIASES> >(map)("http200aliases");
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
        ptr_map_insert< list_option<CURLOPT_QUOTE> >(map)("quote");
        ptr_map_insert< list_option<CURLOPT_POSTQUOTE> >(map)("postquote");
        ptr_map_insert< list_option<CURLOPT_PREQUOTE> >(map)("prequote");
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
        ptr_map_insert< integer_option<CURLOPT_MAX_SEND_SPEED_LARGE> >(map)("maxSendSpeed");
        ptr_map_insert< integer_option<CURLOPT_MAX_RECV_SPEED_LARGE> >(map)("maxRecvSpeed");
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
        // OTHER OPTIONS
        ptr_map_insert< integer_option<CURLOPT_NEW_FILE_PERMS> >(map)("newFilePerms");
        ptr_map_insert< integer_option<CURLOPT_NEW_DIRECTORY_PERMS> >(map)("newDirectoryPerms");
        // TELNET OPTIONS
        ptr_map_insert< list_option<CURLOPT_TELNETOPTIONS> >(map)("telnetoptions");
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
      // TODO: throw exception if option is unkown?
      return false;
    }
  }

  namespace {
    boost::mutex cookie;

    /* To make sure that curl_global_init are never called from parallel
       threads and are paired cleanly. */
    FLUSSPFERD_CLASS_DESCRIPTION(
        cURL_cookie,
        (constructor_name, "$$cURL_cookie")
        (full_name, "cURL.$$cURL_cookie")
        (constructible, false)
    ) {
    public:
      cURL_cookie(object const &obj)
        : base_type(obj)
      {
        boost::mutex::scoped_lock lock(cookie);
        CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
        if(ret != 0)
          throw flusspferd::exception(std::string("curl_global_init: ")
                                      + curl_easy_strerror(ret));
      }

      ~cURL_cookie() {
        boost::mutex::scoped_lock lock(cookie);
        curl_global_cleanup();
      }
    };
  }

  FLUSSPFERD_LOADER_SIMPLE(cURL) {
    local_root_scope scope;

    load_class<cURL_cookie>(cURL);

    create<cURL_cookie>(
        param::_name = "$cURL_cookie",
        param::_container = cURL);

    load_class<EasyOpt>(cURL);
    load_class<Easy>(cURL);

    cURL.define_properties(read_only_property | permanent_property)
        ("version", string(curl_version()))
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
