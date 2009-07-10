// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_SPIDERMONKEY_STRING_HPP
#define FLUSSPFERD_SPIDERMONKEY_STRING_HPP

#include <js/jsapi.h>
#include <cstddef>

namespace flusspferd {

class value;

typedef jschar char16_t;

#ifndef IN_DOXYGEN

namespace Impl {

class string_impl {
  JSString *str;

protected:
  JSString *get()       { return str; }
  void set(JSString *s) { str = s; }

  string_impl();
  string_impl(JSString *s) : str(s) { }
  string_impl(char const *s);
  string_impl(char const *s, std::size_t n);
  string_impl(char16_t const *s, std::size_t n);
  string_impl(value const &v);

  friend JSString *get_string(string_impl &s);
  friend string_impl wrap_string(JSString *s);

public:
  void *get_gcptr() {
    return &str;
  }
};

inline JSString *get_string(string_impl &s) {
  return s.get();
}

inline string_impl wrap_string(JSString *s) {
  return string_impl(s);
}

}

#endif

}

#endif /* FLUSSPFERD_SPIDERMONKEY_STRING_HPP */
