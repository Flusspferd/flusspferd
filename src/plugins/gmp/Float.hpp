// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Rüdiger Sonderfeld

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

#ifndef FLUSSPFERD_PLUGINS_GMP_FLOAT_HPP
#define FLUSSPFERD_PLUGINS_GMP_FLOAT_HPP

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include <gmpxx.h>

namespace multi_precission {
struct Float : public flusspferd::native_object_base {
  struct class_info : flusspferd::class_info {
    static char const *constructor_name() {
      return "Float";
    }

    static char const *full_name() {
      return "Float";
    }

    static object create_prototype() {
      flusspferd::object proto = flusspferd::create_object();
      create_native_method(proto, "fits_int", &Float::fits_int);
      create_native_method(proto, "get_int", &Float::get_int);
      create_native_method(proto, "toInt", &Float::get_int);
      create_native_method(proto, "get_double", &Float::get_double);
      create_native_method(proto, "toDouble", &Float::get_double);
      create_native_method(proto, "get_string", &Float::get_string);
      create_native_method(proto, "toString", &Float::get_string);
      create_native_method(proto, "get_string_base", &Float::get_string_base);
      create_native_method(proto, "get_prec", &Float::get_prec);
      create_native_method(proto, "set_prec", &Float::set_prec);
      create_native_method(proto, "sqrt", &Float::sqrt);
      create_native_method(proto, "sgn", &Float::sgn);
      create_native_method(proto, "abs", &Float::abs);
      create_native_method(proto, "ceil", &Float::ceil);
      create_native_method(proto, "floor", &Float::floor);
      create_native_method(proto, "trunc", &Float::trunc);
      create_native_method(proto, "cmp", &Float::cmp);
      create_native_method(proto, "add", &Float::add);
      create_native_method(proto, "sub", &Float::sub);
      create_native_method(proto, "mul", &Float::mul);
      create_native_method(proto, "div", &Float::div);
      return proto;
    }
  };
  mpf_class mp;

  Float(flusspferd::object const &self, mpf_class const &mp);
  Float(flusspferd::object const &self, flusspferd::call_context &x);

  bool fits_int() /*const*/;
  int get_int() /*const*/;
  double get_double() /*const*/;
  std::string get_string() /*const*/;
  std::string get_string_base(int base) /*const*/;
  int get_prec() /*const*/;
  void set_prec(int p);

  template<typename T>
  Float &create_float(T mp) const {
    return flusspferd::create_native_object<Float>(object(), mpf_class(mp));
  }

  // this should be external but js doesn't support overloading!
  Float &sqrt() /*const*/;
  int sgn() /*const*/;
  Float &abs() /*const*/;
  Float &ceil() /*const*/;
  Float &floor() /*const*/;

  // hypot

  Float &trunc() /*const*/;

  // operators
  void cmp(flusspferd::call_context &x) /*const*/;
  void add(flusspferd::call_context &x) /*const*/;
  void sub(flusspferd::call_context &x) /*const*/;
  void mul(flusspferd::call_context &x) /*const*/;
  void div(flusspferd::call_context &x) /*const*/;
  
private:
  void init_with_value(flusspferd::value v);
};
}

#endif
