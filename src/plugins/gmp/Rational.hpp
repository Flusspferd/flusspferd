// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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
#include "flusspferd/class_description.hpp"
#include <gmpxx.h>

namespace multi_precision {

class Integer;

FLUSSPFERD_CLASS_DESCRIPTION(
  Rational,
  (constructor_name, "Rational")
  (full_name, "gmp.Rational")
  (methods,
    ("get_double",        bind,   get_double) // deprecated
    ("toDouble",          alias,  "get_double")
    ("get_string",        bind,   get_string) // deprecated
    ("toString",          alias,  "get_string")
    ("get_string_base",   alias,  "get_string") // deprecated
    ("sgn",               bind,   sgn)
    ("abs",               bind,   abs)
    ("canonicalize",      bind,   canonicalize)
    ("get_num",           bind,   get_num) // deprecated
    ("get_den",           bind,   get_den) // deprecated
    ("cmp",               bind,   cmp)
    ("add",               bind,   add)
    ("sub",               bind,   sub)
    ("mul",               bind,   mul)
    ("div",               bind,   div))
  (properties,
   ("numerator", getter_setter, (get_num, set_num))
   ("denominator", getter_setter, (get_den, set_den))))
{
public:
  mpq_class mp;

  Rational(flusspferd::object const &self, mpq_class const &mp);
  Rational(flusspferd::object const &self, flusspferd::call_context &x);

  double get_double() /*const*/;
  void get_string(flusspferd::call_context &cc) /*const*/;

  template<typename T>
  static Rational &create_rational(T mp) {
    return flusspferd::create<Rational>(boost::fusion::vector1<mpq_class>(mp));
  }

  int sgn() /*const*/;
  Rational &abs() /*const*/;
  void canonicalize();
  
  Integer &get_num() /*const*/;
  void set_num(Integer &i);
  Integer &get_den() /*const*/;
  void set_den(Integer &i);

  // operators
  void cmp(flusspferd::call_context &x) /*const*/;

  void add(flusspferd::call_context &x) /*const*/;
  void sub(flusspferd::call_context &x) /*const*/;
  void mul(flusspferd::call_context &x) /*const*/;
  void div(flusspferd::call_context &x) /*const*/;
};

}

#endif
