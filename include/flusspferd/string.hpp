// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_STRING_HPP
#define FLUSSPFERD_STRING_HPP

#include "implementation/string.hpp"
#include <string>

namespace flusspferd {
  class value;

  typedef Impl::char16_t char16_t;

  class string : public Impl::string_impl {
  public:
    string();
    string(string const &o);
    string(char const *s);
    string(value const &v);
    string(std::string const &s);
    string(Impl::string_impl const &s)
      : Impl::string_impl(s)
    { }

    ~string();

    string &operator=(string const &o);

    std::size_t length() const;

    std::string to_string() const;
    char const *c_str() const;

    std::basic_string<char16_t> to_utf16_string() const;

    string substr(size_t start, size_t length);
  };

  bool operator==(string const &lhs, string const &rhs);
  bool operator<(string const &lhs, string const &rhs);
}

#endif /* FLUSSPFERD_STRING_HPP */
