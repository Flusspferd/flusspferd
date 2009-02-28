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

#ifndef FLUSSPFERD_VALUE_HPP
#define FLUSSPFERD_VALUE_HPP

#include "implementation/value.hpp"

namespace flusspferd {

class object;
class string;

/**
 * Any Javascript value.
 */
class value : public Impl::value_impl {
private:
  template<typename T>
  value(T *);

public:
  /// Create a new value (Javascript: <code>undefined</code>).
  value();

  /**
   * Create a new boolean value.
   *
   * @param b The value.
   */
  value(bool b);

  /**
   * Create a new number value from an integer.
   *
   * @param i The integer value.
   */
  value(int i);

  /**
   * Create a new number value.
   *
   * @param d The value.
   */
  value(double d);

  /**
   * Create a new object value.
   *
   * @param o The object.
   */
  value(object const &o);

  /**
   * Create a new string value.
   *
   * @param s The string.
   */
  value(string const &s);

  /// Destructor.
  ~value();

#ifndef IN_DOXYGEN
  value(Impl::value_impl const &v)
    : Impl::value_impl(v)
  { }
#endif

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

  /// Check if the value is void (<code>undefined</code>).
  bool is_void() const;

  /// Check if the value is <code>undefined</code> or <code>null</code>.
  bool is_void_or_null() const { return is_null() || is_void(); }

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

#endif /* FLUSSPFERD_VALUE_HPP */
