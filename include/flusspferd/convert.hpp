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
#include <boost/mpl/eval_if.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <limits>

namespace flusspferd {

namespace detail {

template<typename T>
struct convert_arithmetic;

template<typename T>
struct convert {
  typedef typename boost::mpl::eval_if<
      boost::is_arithmetic<T>,
      typename convert_arithmetic<T>::type,
      int
    >::type type;
};

template<>
struct convert<value> {
  typedef convert<value> type;

  static value const &to_value(value const &v) {
    return v;
  }

  static value const &from_value(value const &v) {
    return v;
  }
};

template<>
struct convert<bool> {
  typedef convert<bool> type;

  static value to_value(bool x) {
    return value(x);
  }

  static bool from_value(value const &v) {
    return v.to_boolean();
  }
};

template<>
struct convert<object> {
  typedef convert<object> type;

  static value to_value(object const &o) {
    return value(o);
  }

  static object from_value(value const &v) {
    return v.to_object();
  }
};

template<typename T>
struct convert_arithmetic {
  typedef convert_arithmetic<T> type;

  typedef std::numeric_limits<T> limits;

  static value to_value(T const &x) {
    return value(x);
  };

  static T from_value(value const &v) {
    if (limits::is_integer)
      return v.to_integral_number(limits::digits, limits::is_signed);
    else
      return v.to_number();
  }
};

}

template<typename T>
struct convert : detail::convert<T>::type {};

}

#endif
