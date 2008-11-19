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

class value : public Impl::value_impl {
private:
  template<typename T>
  value(T *);

public:
  value();
  value(bool b);
  value(int i);
  value(double d);
  value(object const &o);
  value(string const &s);
  ~value();

  value(Impl::value_impl const &v)
    : Impl::value_impl(v)
  { }

  void bind(value o);
  void unbind();

  bool is_null() const;
  bool is_void() const;
  bool is_int() const;
  bool is_double() const;
  bool is_number() const;
  bool is_boolean() const;
  bool is_string() const;
  bool is_object() const;

  bool is_bool() const { return is_boolean(); }

  bool get_boolean() const;
  bool get_bool() const { return get_boolean(); }

  int get_int() const;
  double get_double() const;
  object get_object() const;
  string get_string() const;

  string to_string() const;
  double to_number() const;
  double to_integral_number(int bits, bool has_negative) const;
  bool to_boolean() const;
  object to_object() const;

  template<typename Visitor>
  inline void visit(Visitor &v) const;
};

// implementation
template<typename Visitor>
void value::visit(Visitor &v) const {
  if(is_int())
    v(get_int());
  else if(is_double())
    v(get_double());
  else if(is_string())
    v(get_string());
  else if(is_object())
    v(get_object());
  else if(is_boolean())
    v(get_boolean());
  else
    return;
}

}

#endif /* FLUSSPFERD_VALUE_HPP */
