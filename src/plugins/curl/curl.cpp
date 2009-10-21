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
#include "flusspferd/modules.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/class_description.hpp"


#include <curl/curl.h>

using namespace flusspferd;

namespace {
  long global_init(long flags) {
    return curl_global_init(flags);
  }

  FLUSSPFERD_CLASS_DESCRIPTION
  (
   Curl,
   (constructor_name, "CURL")
   (constructible, false)
   (full_name, "cURL.CURL")
   )
  {
    CURL *handle;
  public:
    CURL *data() { return handle; }
    CURL *get() {
      assert(handle);
      return handle;
    }

    Curl(flusspferd::object const &self, CURL *hnd)
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
    ~Curl() { cleanup(); }

    static Curl &create(CURL *hnd) {
      return flusspferd::create_native_object<Curl>(object(), hnd);
    }
  };

  Curl &wrap(CURL *hnd) {
    return Curl::create(hnd);
  }
  CURL *unwrap(Curl &c) {
    return c.data();
  }

  Curl &easy_init() {
    CURL *hnd = curl_easy_init();
    if(!hnd) {
      throw flusspferd::exception("curl_easy_init");
    }
    return wrap(hnd);
  }

  void easy_cleanup(Curl &c) {
    c.cleanup();
  }

  void easy_reset(Curl &c) {
    curl_easy_reset(c.get());
  }

  std::string easy_escape(Curl &c, char const *input) {
    char *esc = curl_easy_escape(c.get(), input, 0);
    if(!esc) {
      throw flusspferd::exception("curl_easy_escape");
    }
    std::string ret(esc);
    curl_free(esc);
    return ret;
  }

  std::string easy_unescape(Curl &c, char const *input) {
    int len;
    char *uesc = curl_easy_unescape(c.get(), input, 0, &len);
    if(!uesc) {
      throw flusspferd::exception("curl_easy_escape");
    }
    std::string ret(uesc, len);
    curl_free(uesc);
    return ret;
  }

  int easy_perform(Curl &c) {
    return curl_easy_perform(c.get());
  }

  void easy_setopt(flusspferd::call_context &x) {
  }

  FLUSSPFERD_LOADER_SIMPLE(cURL) {
    local_root_scope scope;

    cURL.define_property("GLOBAL_ALL", value(CURL_GLOBAL_ALL));
    cURL.define_property("GLOBAL_SSL", value(CURL_GLOBAL_SSL));
    cURL.define_property("GLOBAL_WIN32", value(CURL_GLOBAL_WIN32));
    cURL.define_property("GLOBAL_NOTHING", value(CURL_GLOBAL_NOTHING));
    create_native_function(cURL, "globalInit", &global_init);
    create_native_function(cURL, "version", &curl_version);

    load_class<Curl>(cURL);
    create_native_function(cURL, "easyInit", &easy_init);
    create_native_function(cURL, "easyCleanup", &easy_cleanup);
    create_native_function(cURL, "easyReset", &easy_reset);
    create_native_function(cURL, "easyEscape", &easy_escape);
    create_native_function(cURL, "easyUnescape", &easy_unescape);
    create_native_function(cURL, "easyPerform", &easy_perform);
    //create_native_function(cURL, "easySetopt", &easy_setopt);
    //create_native_function(cURL, "easyGetinfo", &easy_getinfo);
    //...
  }
}
