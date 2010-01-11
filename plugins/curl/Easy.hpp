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
#ifndef FLUSSPFERD_PLUGIN_CURL_EASY_HPP
#define FLUSSPFERD_PLUGIN_CURL_EASY_HPP

#include "exception.hpp"
#include "flusspferd/class_description.hpp"
#include <curl/curl.h>

namespace curl {
  class EasyOpt;

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
    static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream);

    object readfunction_callback;
    static size_t readfunction(void *ptr, size_t size, size_t nmemb, void *stream);

    object seekfunction_callback;
    static int seekfunction(void *stream, curl_off_t offset, int origin);

    object headerfunction_callback;
    static size_t headerfunction(
        void *ptr, size_t size, size_t nmemb, void *stream);

    object progressfunction_callback;
    static int progressfunction(
        void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

    object debugfunction_callback;
    static int debugfunction(
        CURL *hnd, curl_infotype i, char *buf, size_t len, void *p);

  protected:
    void trace(flusspferd::tracer &trc);

  public:
    CURL *data();
    bool valid();
    CURL *get();
    EasyOpt &get_opt();

    Easy(flusspferd::object const &self, flusspferd::call_context&);

    Easy(flusspferd::object const &self, CURL *hnd);

    void cleanup();

    ~Easy();

    void perform();

    void reset();

    std::string unescape(char const *input);

    std::string escape(char const *input);

    static Easy &create(CURL *hnd);

    //private:
    template<typename T>
    void do_setopt(CURLoption what, T data) {
      CURLcode res = curl_easy_setopt(get(), what, data);
      if(res != 0) {
        throw exception(std::string("curl_easy_setopt: ") +
                        curl_easy_strerror(res));
      }
    }
  };

  Easy &wrap(CURL *hnd);
  CURL *unwrap(Easy &c);
}

#endif
