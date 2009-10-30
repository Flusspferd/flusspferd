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
#include <boost/version.hpp>

#include <iostream>

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
#if BOOST_VERSION < 103900
			boost::shared_ptr<CURLcode const*> code;
#else
			CURLcode const *code;
#endif
			if( (code = ::boost::get_error_info<curlcode_info>(*this)) ) {
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

	namespace {
		struct handle_option {
			virtual ~handle_option() =0;
			virtual value default_value() const { return value(); }
			virtual void handle(Easy &e, value const &v) const =0;
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
    ("valid",    bind, valid))
	 (properties,
		("options", getter, get_opt))
   )
  {
    CURL *handle;
		EasyOpt &opt;

		static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream) {
			assert(stream);
			Easy &self = *reinterpret_cast<Easy*>(stream);
			byte_array data(object(),
											reinterpret_cast<byte_array::element_type*>(ptr),
											size*nmemb);
			arguments arg;
			arg.push_back(value(data));
			arg.push_back(value(size));
			object callback = self.opt.get_property("writefunction").to_object();
			value v = callback.call(arg);
			return v.to_number();
		}

	protected:
		void trace(flusspferd::tracer &trc) {
			trc("options", opt);
			trc("defaultFunction", default_function);
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

		/**
		 * Applies all options set in EasyOpt.
		 * This is called automatically.
		 * Shouldn't be necessary to call it manually!
		 *
		 * Not exported.
		 *
		 * Maybe this should be called by unwrap and private ...
		 */
		void apply_options() {
			std::cout << "apply_options()\n";
			options_map_t const &options = get_options();
			for(property_iterator i = opt.begin();
					i != opt.end();
					++i)
			{
				std::cout << "\t" << i->to_std_string() << "\n";
				options_map_t::const_iterator option = options.find(i->to_std_string());
				if(option != options.end()) {
					option->second->handle(*this, opt.get_property(*i));
				}
			}
		}

    void perform() {
			apply_options();
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

    static Easy &create(CURL *hnd) {
      return flusspferd::create_native_object<Easy>(object(), hnd);
    }

		static object default_function;

		//private:
		template<typename T>
		void do_setopt(CURLoption what, T data) {
			std::cout << "setopt (" << (unsigned)what << ' ' << data << '\n'; // DEBUG
			CURLcode res = curl_easy_setopt(get(), what, data);
			if(res != 0) {
				throw flusspferd::exception(std::string("curl_easy_setopt: ") +
																		curl_easy_strerror(res));
			}
		}

		void do_setopt_function(CURLoption what, bool default_) {
			switch(what) {
			case CURLOPT_WRITEFUNCTION:
				do_setopt(CURLOPT_WRITEFUNCTION, default_ ? 0x0 : &writefunction);
				do_setopt(CURLOPT_WRITEDATA, this);
				break;
			default:
				throw flusspferd::exception("unkown function option");
			};
		}
  };
	object Easy::default_function;

  Easy &wrap(CURL *hnd) {
    return Easy::create(hnd);
  }
  CURL *unwrap(Easy &c) {
    return c.data();
  }

	namespace {
		struct integer_option : handle_option {
			integer_option(CURLoption what) : what(what) { }
			value default_value() const { return value(0); }
			void handle(Easy &e, value const &v) const {
				e.do_setopt(what, v.to_number());
			}
  	private:
			CURLoption what;
		};

		struct string_option : handle_option {
			string_option(CURLoption what) : what(what) { }
			value default_value() const { return value(""); }
			void handle(Easy &e, value const &v) const {
				e.do_setopt(what, v.to_std_string().c_str());
			}
		private:
			CURLoption what;
		};

		struct function_option : handle_option {
			function_option(CURLoption what) : what(what) { }
			value default_value() const { return value(Easy::default_function); }
			void handle(Easy &e, value const &v) const {
				e.do_setopt_function(what, v == default_value());
			}
		private:
			CURLoption what;
		};

		options_map_t const &get_options() {
			static options_map_t map;
			if(map.empty()) {
				boost::assign::ptr_map_insert<integer_option>(map)
					("verbose", CURLOPT_VERBOSE)
					("header", CURLOPT_HEADER)
					("noprogress", CURLOPT_NOPROGRESS)
					("nosignal", CURLOPT_NOSIGNAL);
				boost::assign::ptr_map_insert<function_option>(map)
					("writefunction", CURLOPT_WRITEFUNCTION);
				boost::assign::ptr_map_insert<string_option>(map)
					("url", CURLOPT_URL);
			}
			return map;
		}
	}

	bool EasyOpt::property_resolve(value const &id, unsigned) {
		std::string name = id.to_std_string();
		options_map_t::const_iterator i;
		if( (i = get_options().find(name)) != get_options().end() ) {
			define_property(id.get_string(), i->second->default_value());
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
    create_native_function(cURL, "globalInit", &global_init);
    cURL.define_property("version", value(curl_version()),
                         read_only_property | permanent_property);
		load_class<EasyOpt>(cURL);
    load_class<Easy>(cURL);
		cURL.define_property("defaultFunction", value(Easy::default_function),
												 read_only_property | permanent_property);
  }
}
