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
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_float.hpp>
#include <boost/type_traits/is_integral.hpp>
//#include <boost/mpl/set.hpp>               <- may be useful soon
//#include <boost/mpl/has_key.hpp>
#include <limits>

namespace flusspferd {

namespace detail {

template<typename T, typename Condition = void>
struct convert;

template<class T, class C = T>
struct to_value_helper {
  value perform(T const &x) {
    return value(C(x));
  }
};

template<class T>
struct to_value_helper<T, T> {
  value perform(T const &x) {
    return value(x);
  }
};

template<>
struct convert<value, void> {
  struct to_value {
    value const &perform(value const &v) {
      return v;
    }
  };

  typedef to_value from_value;
};

template<>
struct convert<bool, void> {
  typedef to_value_helper<bool> to_value;

  struct from_value {
    bool perform(value const &v) {
      return v.get_boolean();
    }
  };
};

template<>
struct convert<object, void>
{
  typedef to_value_helper<object> to_value;

  struct from_value {
    root_value root;

    object perform(value const &v) {
      object o = v.to_object();
      root = o;
      return o;
    }
  };
};

template<>
struct convert<string, void> {
  typedef to_value_helper<string> to_value;

  struct from_value {
    root_value root;

    string perform(value const &v) {
      string s = v.to_string();
      root = s;
      return s;
    }
  };
};

template<>
struct convert<function, void> {
  typedef to_value_helper<function> to_value;

  struct from_value {
    root_value root;

    function perform(value const &v) {
      function f = function(v.to_object());
      root = object(f);
      return f;
    }
  };
};

template<>
struct convert<char const *, void> {
  typedef to_value_helper<char const *, string> to_value;

  struct from_value {
    root_value root;

    char const *perform(value const &v) {
      string s = v.to_string();
      root = s;
      return s.c_str();
    }
  };
};

template<>
struct convert<std::string, void> {
  typedef to_value_helper<std::string, string> to_value;

  struct from_value {
    std::string perform(value const &v) {
      return v.to_string().to_string();
    }
  };
};

template<>
struct convert<std::basic_string<char16_t>, void> {
  typedef std::basic_string<char16_t> string_t;

  typedef to_value_helper<string_t, string> to_value;

  struct from_value {
    string_t perform(value const &v) {
      return v.to_string().to_utf16_string();
    }
  };
};

template<typename T>
struct convert<
    T,
    typename boost::enable_if<boost::is_integral<T> >::type
  >
{
  typedef to_value_helper<T, double> to_value;

  struct from_value {
    typedef std::numeric_limits<T> limits;

    T perform(value const &v) {
      return v.to_integral_number(limits::digits, limits::is_signed);
    }
  };
};

template<typename T>
struct convert<
    T,
    typename boost::enable_if<boost::is_float<T> >::type
  >
{
  typedef to_value_helper<T, double> to_value;

  struct from_value {
    T perform(value const &v) {
      return v.to_number();
    }
  };
};

}

template<typename T>
class convert : private detail::convert<T>, private boost::noncopyable {
private:
  typedef detail::convert<T> base;

public:
  typedef typename base::to_value to_value;
  typedef typename base::from_value from_value;
};

}

#endif
