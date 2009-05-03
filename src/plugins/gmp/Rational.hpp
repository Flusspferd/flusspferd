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

#ifndef FLUSSPFERD_PLUGINS_GMP_RATIONAL_HPP
#define FLUSSPFERD_PLUGINS_GMP_RATIONAL_HPP

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include <gmpxx.h>

namespace multi_precission {
struct Rational : public flusspferd::native_object_base {
  struct class_info : flusspferd::class_info {
    static char const *constructor_name() {
      return "Rational";
    }

    static char const *full_name() {
      return "Rational";
    }

    static object create_prototype() {
      flusspferd::object proto = flusspferd::create_object();
      // create_native_method(proto, "fits_int", &Rational::fits_int);
      // create_native_method(proto, "get_int", &Rational::get_int);
      // create_native_method(proto, "get_double", &Rational::get_double);
      // create_native_method(proto, "get_string", &Rational::get_string);
      // create_native_method(proto, "get_string_base", &Rational::get_string_base);
      // create_native_method(proto, "get_prec", &Rational::get_prec);
      // create_native_method(proto, "set_prec", &Rational::set_prec);
      // create_native_method(proto, "sqrt", &Rational::sqrt);
      // create_native_method(proto, "sgn", &Rational::sgn);
      // create_native_method(proto, "abs", &Rational::abs);
      // create_native_method(proto, "ceil", &Rational::ceil);
      // create_native_method(proto, "floor", &Rational::floor);
      // create_native_method(proto, "trunc", &Rational::trunc);
      // create_native_method(proto, "cmp", &Rational::cmp);
      // create_native_method(proto, "add", &Rational::add);
      // create_native_method(proto, "sub", &Rational::sub);
      // create_native_method(proto, "mul", &Rational::mul);
      // create_native_method(proto, "div", &Rational::div);
      return proto;
    }
  };
  mpq_class mp;

  Rational(flusspferd::object const &self, mpq_class const &mp);
  Rational(flusspferd::object const &self, flusspferd::call_context &x);
};
}

#endif
