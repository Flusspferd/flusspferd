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
#include "flusspferd/tracer.hpp"
#include "flusspferd/modules.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/class_description.hpp"

#include <sstream>
#include <curl/curl.h>

using namespace flusspferd;

namespace {
  void global_init(long flags) {
    CURLcode ret = curl_global_init(flags);
		if(ret != 0) {
			throw flusspferd::exception(std::string("curl_global_init: ") +
																	curl_easy_strerror(ret));
		}
  }

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
   )
  {
    CURL *handle;

		object writecallback;
		static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream) {
			assert(stream);
			Easy &self = *reinterpret_cast<Easy*>(stream);
			//			self.writecallback.call();
			// TODO ...
			return size * nmemb;
		}
	protected:
		void trace(flusspferd::tracer &trc) {
			trc("writecallback", writecallback);
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

    Easy(flusspferd::object const &self, flusspferd::call_context&)
      : base_type(self), handle(curl_easy_init())
    {
      if(!handle) {
        throw flusspferd::exception("curl_easy_init");
      }
    }

    Easy(flusspferd::object const &self, CURL *hnd)
      : base_type(self), handle(hnd)
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

    int perform() {
      return curl_easy_perform(get());
    }

    void reset() {
      curl_easy_reset(get());
    }

    std::string unescape(char const *input) {
      int len;
      char *uesc = curl_easy_unescape(get(), input, 0, &len);
      if(!uesc) {
        throw flusspferd::exception("curl_easy_escape");
      }
      std::string ret(uesc, len);
      curl_free(uesc);
      return ret;
    }

    std::string escape(char const *input) {
      char *esc = curl_easy_escape(get(), input, 0);
      if(!esc) {
        throw flusspferd::exception("curl_easy_escape");
      }
      std::string ret(esc);
      curl_free(esc);
      return ret;
    }

#define OPT_NUMBER(what)                                                \
    case what :                                                         \
      if(x.arg.size() != 2) {                                           \
        std::stringstream ss;                                           \
        ss << "curl_easy_setopt: Expected two arguments. Got (" << x.arg.size() << ')'; \
        throw flusspferd::exception(ss.str());                          \
      }                                                                 \
      int res = curl_easy_setopt(get(), what , x.arg[1].to_number());		\
			if(res != 0) {																										\
				throw flusspferd::exception(std::string("curl_easy_setopt: ") +	\
																		curl_easy_strerror(res));						\
			}																																	\
      break;                                                            \
      /**/

#define OPT_FUNCTION(what, data, callback, func)

    void setopt(flusspferd::call_context &x) {
      int what = x.arg.front().to_number();
      switch(what) {
        //OPT_NUMBER(CURLOPT_HEADER)
      case CURLOPT_WRITEFUNCTION: {
				// TODO reset to default function (0x0 parameter!)
				if(!x.arg[1].is_object()) {
					throw flusspferd::exception("curl_easy_setopt: expected a function as second parameter!");
				}
				writecallback = x.arg[1].to_object();
        int res = curl_easy_setopt(get(), CURLOPT_WRITEDATA, this);
        if(res != 0) {
          throw flusspferd::exception(std::string("curl_easy_setopt: ") +
																			curl_easy_strerror((CURLcode)res));
        }
				res = curl_easy_setopt(get(), CURLOPT_WRITEFUNCTION, writefunction);
				if(res != 0) {
					throw flusspferd::exception(std::string("curl_easy_setopt: ") +
																			curl_easy_strerror((CURLcode)res));
				}
			}
      default: {
        std::stringstream ss;
        ss << "curl_easy_setopt unkown or unsupported option (" << what << ')';
        throw flusspferd::exception(ss.str());
      }
      };
    }

#undef OPT_FUNCTION
#undef OPT_NUMBER

    static Easy &create(CURL *hnd) {
      return flusspferd::create_native_object<Easy>(object(), hnd);
    }
  };

  Easy &wrap(CURL *hnd) {
    return Easy::create(hnd);
  }
  CURL *unwrap(Easy &c) {
    return c.data();
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

    load_class<Easy>(cURL);
    cURL.define_property("OPT_HEADER", value((int)CURLOPT_HEADER),
                         read_only_property | permanent_property);
		cURL.define_property("OPT_WRITEFUNCTION", value((int)CURLOPT_WRITEFUNCTION),
                         read_only_property | permanent_property);
  }
}
