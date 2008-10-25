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

#ifndef FLUSSPFERD_CONVERT_HPP
#define FLUSSPFERD_CONVERT_HPP

#include "value.hpp"
#include "object.hpp"
#include "string.hpp"
#include "function.hpp"
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <limits>

namespace flusspferd {

namespace detail {

template<typename T>
struct convert_arithmetic;

template<typename T>
struct convert 
: boost::mpl::if_<
    boost::is_arithmetic<T>,
    convert_arithmetic<T>,
    int
  >::type
{};

template<>
struct convert<value> {
  static value const &to_value(value const &v) {
    return v;
  }

  static value const &from_value(value const &v, value&) {
    return v;
  }
};

template<>
struct convert<bool> {
  static value to_value(bool x) {
    return value(x);
  }

  static bool from_value(value const &v, value&) {
    return v.to_boolean();
  }
};

template<>
struct convert<object> {
  static value to_value(object const &o) {
    return value(o);
  }

  static object from_value(value const &v, value &root) {
    object o = v.to_object();
    root = value(o);
    return o;
  }
};

template<>
struct convert<string> {
  static value to_value(string const &x) {
    return value(x);
  }

  static string from_value(value const &v, value &root) {
    string s = v.to_string();
    root = value(s);
    return s;
  }
};

template<>
struct convert<function> {
  static value to_value(function const &x) {
    return value(object(x));
  }

  static function from_value(value const &v, value &root) {
    function f(v.to_object());
    root = value(object(f));
    return f;
  }
};

template<>
struct convert<char const *> {
  static value to_value(char const *x) {
    return value(string(x));
  }

  static char const *from_value(value const &v, value &root) {
    string s = v.to_string();
    root = value(s);
    return s.c_str();
  }
};

template<>
struct convert<std::string> {
  static value to_value(std::string const &x) {
    return value(string(x));
  }

  static std::string from_value(value const &v, value &) {
    string s = v.to_string();
    return s.to_string();
  }
};

template<>
struct convert<std::basic_string<char16_t> > {
  typedef std::basic_string<char16_t> string_t;

  static value to_value(string_t const &x) {
    return value(string(x));
  }

  static string_t from_value(value const &v, value &) {
    string s = v.to_string();
    return s.to_utf16_string();
  }
};

template<typename T>
struct convert_arithmetic {
  typedef std::numeric_limits<T> limits;

  static value to_value(T const &x) {
    return value(x);
  };

  static T from_value(value const &v, value&) {
    if (limits::is_integer)
      return v.to_integral_number(limits::digits, limits::is_signed);
    else
      return v.to_number();
  }
};

}

template<typename T>
struct convert : private detail::convert<T> {
  using detail::convert<T>::to_value;
  using detail::convert<T>::from_value;
};

}

#endif
