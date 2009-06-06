// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
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

#ifndef FLUSSPFERD_VALUE_HPP
#define FLUSSPFERD_VALUE_HPP

#include "spidermonkey/value.hpp"
#include <string>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/logical.hpp>

namespace flusspferd {

class object;
class string;
template<typename T> class convert;

/**
 * Any Javascript value.
 *
 * @ingroup value_types
 */
class value : public Impl::value_impl {
public:
  /// Create a new value (Javascript: <code>undefined</code>).
  value();

  /// Copy constructor.
  value(value const &o)
  : Impl::value_impl(o)
  {}

#ifndef IN_DOXYGEN
  template<typename BoolType>
  explicit value(
    BoolType const &x,
    typename boost::enable_if<
      typename boost::is_same<BoolType, bool>::type
    >::type * = 0)
  : Impl::value_impl(Impl::value_impl::from_boolean(x))
  {}

  template<typename IntegralType>
  explicit value(
    IntegralType const &num,
    typename boost::enable_if<
      typename boost::mpl::and_<
        typename boost::is_integral<IntegralType>::type,
        typename boost::mpl::not_<
          typename boost::is_same<IntegralType, bool>::type
        >::type
      >::type
    >::type * = 0)
  : Impl::value_impl(Impl::value_impl::from_integer(num))
  {}

  template<typename FloatingPointType>
  explicit value(
    FloatingPointType const &num,
    typename boost::enable_if<
      typename boost::is_floating_point<FloatingPointType>::type
    >::type * = 0)
  : Impl::value_impl(Impl::value_impl::from_double(num))
  {}

  template<typename OtherType>
  explicit value(
    OtherType const &val,
    typename boost::disable_if<
      typename boost::mpl::or_<
        typename boost::is_integral<OtherType>::type,
        typename boost::is_floating_point<OtherType>::type,
        typename boost::is_convertible<OtherType, object>::type
      >::type
    >::type * = 0)
  {
    typename convert<OtherType>::to_value converter;
    *this = converter.perform(val);
  }
#else
  /**
   * Create a new boolean value.
   *
   * @param b The value.
   */
  explicit value(bool b);

  /**
   * Create a new number value from an integer.
   *
   * @param num The integer value.
   */
  template<typename IntegralType>
  explicit value(IntegralType const &num);

  /**
   * Create a new number value from a floating point number.
   *
   * @param num The floating point value.
   */
  template<typename FloatingPointType>
  explicit value(FloatingPointType const &num);

  /**
   * Create a new value from a (nearly) arbitrary C++ type value.
   *
   * Works with any type supported by flusspferd::convert. This overload will
   * not be used for integral types, floating point types or types convertible
   * to flusspferd::object.
   *
   * @param val The value to convert.
   */
  template<typename OtherType>
  explicit value(OtherType const &val);
#endif

  /**
   * Create a new object value.
   *
   * @param o The object.
   */
  value(object const &o)
  : Impl::value_impl(Impl::value_impl::from_object(o))
  {}

  /**
   * Create a new string value.
   *
   * @param s The string.
   */
  value(string const &s)
  : Impl::value_impl(Impl::value_impl::from_string(s))
  {}

  /// Destructor.
  ~value();

#ifndef IN_DOXYGEN
  value(Impl::value_impl const &v)
    : Impl::value_impl(v)
  { }
#endif

  // Assignment operator.
  value &operator=(value const &v) {
    *static_cast<Impl::value_impl*>(this) = v;
    return *this;
  }

  // Assignment operator.
  template<typename T>
  value &operator=(T const &x) {
    *this = value(x);
    return *this;
  }

  /**
   * Bind to another value.
   *
   * The value will act as a reference to @p o.
   *
   * @param o The value to bind to.
   */
  void bind(value o);

  /**
   * Unbind the value.
   *
   * If the value is a reference to another value, the binding will be removed.
   * Also, the value will be reset to (Javascript) <code>undefined</code>.
   */
  void unbind();

  /// Check if the value is <code>null</code>.
  bool is_null() const;

  // Check if the value is <code>undefined</code>.  
  bool is_undefined() const;
  
  // Check if the value is <code>undefined</code> or <code>null</code>.
  bool is_undefined_or_null() const { return is_null() || is_undefined(); }

  /// Check if the value is an int.
  bool is_int() const;

  /// Check if the value is a double.
  bool is_double() const;

  /// Check if the value is a number.
  bool is_number() const;

  /// Check if the value is boolean.
  bool is_boolean() const;

  /// Check if the value is a string.
  bool is_string() const;

  /// Check if the value is an object.
  bool is_object() const;

  /// Check if the value is a function.
  bool is_function() const;

  /// Check if the value is boolean.
  bool is_bool() const { return is_boolean(); }

  /// Get the boolean value.
  bool get_boolean() const;

  /// Get the boolean value.
  bool get_bool() const { return get_boolean(); }

  /// Get the int value.
  int get_int() const;

  /// Get the double value.
  double get_double() const;

  /// Get the object value.
  object get_object() const;

  /// Get the string value.
  string get_string() const;

  /// Convert the value to a string.
  string to_string() const;

  /// Convert the value to a C++ standard library string (std::string)
  std::string to_std_string() const;

  /// Convert the number to a number.
  double to_number() const;

  /// Convert the value to an integral number.
  double to_integral_number(int bits, bool has_negative) const;

  /// Convert the value to a boolean.
  bool to_boolean() const;

  /// Convert the value to an object.
  object to_object() const;
};

}

#include "convert.hpp"

#endif /* FLUSSPFERD_VALUE_HPP */
