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
#ifndef FLUSSPFERD_PLUGIN_CURL_STRING_OPTION_HPP
#define FLUSSPFERD_PLUGIN_CURL_STRING_OPTION_HPP

#include "handle_option.hpp"
#include "EasyOpt.hpp"
#include "Easy.hpp"
#include "flusspferd/create/function.hpp"

namespace curl {
  template<CURLoption What>
  struct string_option : handle_option {
    flusspferd::object getter() const {
      return flusspferd::create<flusspferd::method>("$get_", &get);
    }
    flusspferd::object setter() const {
      return flusspferd::create<flusspferd::method>("$set_", &set);
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
      o->parent.do_setopt(
          What,
          boost::any_cast<std::string&>(o->data[What]).c_str());
    }
  };
}

#endif
