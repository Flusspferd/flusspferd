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
#include "root_value.hpp"
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
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
  value const &to_value(value const &v) {
    return v;
  }

  value const &from_value(value const &v) {
    return v;
  }
};

template<>
struct convert<bool> {
  value to_value(bool x) {
    return value(x);
  }

  bool from_value(value const &v) {
    return v.to_boolean();
  }
};

template<>
struct convert<object> {
  value to_value(object const &o) {
    return value(o);
  }

  object from_value(value const &v) {
    object o = v.to_object();
    root = boost::in_place(value(o));
    return o;
  }

  boost::optional<root_value> root;
};

template<>
struct convert<string> {
  value to_value(string const &x) {
    return value(x);
  }

  string from_value(value const &v) {
    string s = v.to_string();
    root = boost::in_place(value(s));
    return s;
  }

  boost::optional<root_value> root;
};

template<>
struct convert<function> {
  value to_value(function const &x) {
    return value(object(x));
  }

  function from_value(value const &v) {
    function f(v.to_object());
    root = boost::in_place(value(object(f)));
    return f;
  }

  boost::optional<root_value> root;
};

template<>
struct convert<char const *> {
  value to_value(char const *x) {
    return value(string(x));
  }

  char const *from_value(value const &v) {
    string s = v.to_string();
    root = boost::in_place(value(s));
    return s.c_str();
  }

  boost::optional<root_value> root;
};

template<>
struct convert<std::string> {
  value to_value(std::string const &x) {
    return value(string(x));
  }

  std::string from_value(value const &v) {
    string s = v.to_string();
    return s.to_string();
  }
};

template<>
struct convert<std::basic_string<char16_t> > {
  typedef std::basic_string<char16_t> string_t;

  value to_value(string_t const &x) {
    return value(string(x));
  }

  string_t from_value(value const &v) {
    string s = v.to_string();
    return s.to_utf16_string();
  }
};

template<typename T>
struct convert_arithmetic {
  typedef std::numeric_limits<T> limits;

  value to_value(T const &x) {
    return value(x);
  };

  T from_value(value const &v) {
    if (limits::is_integer)
      return v.to_integral_number(limits::digits, limits::is_signed);
    else
      return v.to_number();
  }
};

}

template<typename T>
struct convert : private detail::convert<T>, private boost::noncopyable {
  using detail::convert<T>::to_value;
  using detail::convert<T>::from_value;
};

}

#endif
