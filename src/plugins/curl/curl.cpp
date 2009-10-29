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

#include <sstream>
#include <curl/curl.h>

#include <boost/exception/get_error_info.hpp>

using namespace flusspferd;

namespace {
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
			throw flusspferd::exception(std::string("curl_global_init: ") + curl_easy_strerror(ret));
		}
  }

	class Easy;

	FLUSSPFERD_CLASS_DESCRIPTION
	(
	 EasyOpt,
	 (constructor_name, "EasyOpt")
	 (full_name, "cURL.Easy.EasyOpt")
	 (constructible, false)
	 )
	{
		Easy &parent;
	public:
		EasyOpt(flusspferd::object const &self, Easy &parent)
			: base_type(self), parent(parent)
		{	}

		static EasyOpt &create(Easy &p) {
			return flusspferd::create_native_object<EasyOpt>(object(), boost::ref(p));
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
    ("setopt",   bind, setopt)
    ("valid",    bind, valid))
	 (properties,
		("options", getter, get_opt))
   )
  {
    CURL *handle;
		EasyOpt &opt;

		object writecallback;
		static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream) {
			assert(stream);
			Easy &self = *reinterpret_cast<Easy*>(stream);
			byte_array data(object(),
											reinterpret_cast<byte_array::element_type*>(ptr),
											size*nmemb);
			arguments arg;
			arg.push_back(value(data));
			arg.push_back(value(size));
			value v = self.writecallback.call(arg);
			return v.to_number();
		}
	protected:
		void trace(flusspferd::tracer &trc) {
			trc("writecallback", writecallback);
			trc("options", opt);
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
				throw flusspferd::exception(std::string("curl_easy_setopt: ") +
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
        throw flusspferd::exception("curl_easy_escape");
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

#define OPT_NUMBER(What)                                                \
    case What :                                                         \
      if(x.arg.size() != 2) {                                           \
        std::stringstream ss;                                           \
        ss << "curl_easy_setopt: Expected two arguments. Got (" << x.arg.size() << ')'; \
        throw flusspferd::exception(ss.str());                          \
      }                                                                 \
      do_setopt(What, x.arg[1].to_number());														\
      break

#define OPT_FUNCTION(What, Data, Callback, Func)												\
		case What : {																												\
			if(x.arg.size() != 2) {																						\
				throw flusspferd::exception("curl_easy_setopt: Expected two arguments"); \
			}																																	\
			if(!x.arg[1].is_object()) {																				\
				throw flusspferd::exception("curl_easy_setopt: Expected a function as second parameter");	\
			}																																	\
			object callback = x.arg[1].get_object();													\
			if(callback == default_function) {																\
				do_setopt(What, 0x0);																						\
				Callback = object();																						\
			}																																	\
			else {																														\
				do_setopt(Data, this);																					\
				do_setopt(What, Func);																					\
				Callback = callback;																						\
			}																																	\
		}																																		\
		break

#define OPT_DATAFUNCTION(What, Prefix)																	\
		OPT_FUNCTION( What ## FUNCTION, What ## DATA, Prefix ## callback, Prefix ## function )

	private:
		template<typename T>
		void do_setopt(CURLoption what, T data) {
		  CURLcode res = curl_easy_setopt(get(), what, data);
			if(res != 0) {
				throw flusspferd::exception(std::string("curl_easy_setopt: ") +
																		curl_easy_strerror(res));
			}
		}
	public:

    void setopt(flusspferd::call_context &x) {
      int what = x.arg.front().to_number();
      switch(what) {
				// Behaviour Options
				OPT_NUMBER(CURLOPT_VERBOSE);
        OPT_NUMBER(CURLOPT_HEADER);
				OPT_NUMBER(CURLOPT_NOPROGRESS);
				OPT_NUMBER(CURLOPT_NOSIGNAL);

				// Callback Options
				OPT_DATAFUNCTION(CURLOPT_WRITE, write);
      default: {
        std::stringstream ss;
        ss << "curl_easy_setopt unkown or unsupported option (" << what << ')';
        throw flusspferd::exception(ss.str());
      }
      };
    }

#undef OPT_DATAFUNCTION
#undef OPT_FUNCTION
#undef OPT_NUMBER

    static Easy &create(CURL *hnd) {
      return flusspferd::create_native_object<Easy>(object(), hnd);
    }

		static object default_function;
  };

	object Easy::default_function;

  Easy &wrap(CURL *hnd) {
    return Easy::create(hnd);
  }
  CURL *unwrap(Easy &c) {
    return c.data();
  }

	bool EasyOpt::property_resolve(value const &id, unsigned) {
		return false;
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
    create_native_function(cURL, "globalInit", &global_init);
    cURL.define_property("version", value(curl_version()),
                         read_only_property | permanent_property);
		load_class<EasyOpt>(cURL);
    load_class<Easy>(cURL);
		// Behaviour Options
		cURL.define_property("OPT_VERBOSE", value((int)CURLOPT_VERBOSE),
												 read_only_property | permanent_property);
    cURL.define_property("OPT_HEADER", value((int)CURLOPT_HEADER),
                         read_only_property | permanent_property);
		cURL.define_property("OPT_NOPROGRESS", value((int)CURLOPT_NOPROGRESS),
												 read_only_property | permanent_property);
		cURL.define_property("OPT_NOSIGNAL", value((int)CURLOPT_NOSIGNAL),
												 read_only_property | permanent_property);
		// Callback Options
		cURL.define_property("OPT_WRITEFUNCTION", value((int)CURLOPT_WRITEFUNCTION),
                         read_only_property | permanent_property);
		/*
		CURLOPT_READFUNCTION
		CURLOPT_IOCTLFUNCTION
		CURLOPT_SEEKFUNCTION
		CURLOPT_SOCKOPTFUNCTION
		CURLOPT_OPENSOCKETFUNCTION
		...
		 */
		cURL.define_property("defaultFunction", value(Easy::default_function),
												 read_only_property | permanent_property);
  }
}
