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
#ifndef FLUSSPFERD_PLUGIN_CURL_FUNCTION_OPTION_HPP
#define FLUSSPFERD_PLUGIN_CURL_FUNCTION_OPTION_HPP

#include "handle_option.hpp"
#include "EasyOpt.hpp"
#include "Easy.hpp"
#include "flusspferd/create/function.hpp"

namespace curl {
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

  template<CURLoption What, CURLoption WhatData, flusspferd::object (Easy::*Obj)>
  struct function_option : handle_option {
    flusspferd::object getter() const {
      return flusspferd::create<flusspferd::method>("$get_", &get);
    }
    flusspferd::object setter() const {
      return flusspferd::create<flusspferd::method>("$set_", &set);
    }
    boost::any data() const { return flusspferd::object(); }
    CURLoption what() const { return What; }
  private:
    static flusspferd::object get(EasyOpt *o) {
      assert(o);
      return o->parent.*(Obj);
    }
    static void set(EasyOpt *o, flusspferd::object val) {
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
}
#endif
