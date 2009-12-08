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
			//std::cout << "writefunction" << std::endl;
			assert(stream);
			Easy &self = *reinterpret_cast<Easy*>(stream);
			if(self.writefunction_callback.is_null()) {
				//std::cout << "writefunction*1" << std::endl;
				return 0;
			}
			else {
				//std::cout << "writefunction^2" << std::endl;
				byte_array data(object(),
												reinterpret_cast<byte_array::element_type*>(ptr),
												size*nmemb);
				arguments arg;
				arg.push_back(value(data));
				arg.push_back(value(size));
				//std::cout << "writefunction^3" << std::endl;
				value v = self.writefunction_callback.call(arg);
				//std::cout << "writefunction^4" << std::endl;
				return v.to_number();
			}
		}

	protected:
		void trace(flusspferd::tracer &trc) {
			trc("options", opt);
			trc("writeFunction", writefunction_callback);
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

		struct write_function_option : handle_option {
			static CURLoption const What = CURLOPT_WRITEFUNCTION;
			function getter() const {
				return create<flusspferd::method>("$get_writeFunction", &get);
			}
			function setter() const {
				return create<flusspferd::method>("$set_writeFunction", &set);
			}
			boost::any data() const { return function(); }
			CURLoption what() const { return What; }
		private:
			static object get(EasyOpt *o) {
				assert(o);
				return o->parent.writefunction_callback;
			}
			static void set(EasyOpt *o, object val) {
				assert(o);
				o->parent.writefunction_callback = val;
				if(val.is_null()) {
					o->parent.do_setopt(CURLOPT_WRITEFUNCTION, 0x0);
				}
				else {
					o->parent.do_setopt(CURLOPT_WRITEFUNCTION, &Easy::writefunction);
					o->parent.do_setopt(CURLOPT_WRITEDATA, &o->parent);
				}
			}
		};

		options_map_t const &get_options() {
			static options_map_t map;
			if(map.empty()) {
				using namespace boost::assign;
				ptr_map_insert< integer_option<CURLOPT_VERBOSE> >(map)
					("verbose");
				ptr_map_insert< integer_option<CURLOPT_HEADER> >(map)
					("header");
				ptr_map_insert< integer_option<CURLOPT_NOPROGRESS> >(map)
					("noProgress");
				ptr_map_insert< integer_option<CURLOPT_NOSIGNAL> >(map)
					("noSignal");
				ptr_map_insert< write_function_option >(map)
					("writeFunction");
				ptr_map_insert< string_option<CURLOPT_URL> >(map)("url");
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
  }
}
