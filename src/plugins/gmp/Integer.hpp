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

#ifndef FLUSSPFERD_PLUGINS_GMP_INTEGER_HPP
#define FLUSSPFERD_PLUGINS_GMP_INTEGER_HPP

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/class_description.hpp"
#include <gmpxx.h>

namespace multi_precission {

FLUSSPFERD_CLASS_DESCRIPTION(
  Integer,
  (constructor_name, "Integer")
  (full_name, "gmp.Integer")
  (methods,
    ("fits_int",         bind,   fits_int)
    ("get_int",          bind,   get_int)
    ("toInt",            alias,  "get_int")
    ("get_double",       bind,   get_double)
    ("toDouble",         alias,  "get_double")
    ("get_string",       bind,   get_string)
    ("toString",         alias,  "get_string")
    ("get_string_base",  bind,   get_string_base)
    ("sqrt",             bind,   sqrt)
    ("sgn",              bind,   sgn)
    ("abs",              bind,   abs)
    ("cmp",              bind,   cmp)
    ("add",              bind,   add)
    ("sub",              bind,   sub)
    ("mul",              bind,   mul)
    ("div",              bind,   div)))
{
public:
  mpz_class mp;

  Integer(flusspferd::object const &self, mpz_class const &mp);
  Integer(flusspferd::object const &self, flusspferd::call_context &x);

  bool fits_int() /*const*/;
  int get_int() /*const*/;
  double get_double() /*const*/;
  std::string get_string() /*const*/;
  std::string get_string_base(int base) /*const*/;

  template<typename T>
  static Integer &create_integer(T mp) {
    return flusspferd::create_native_object<Integer>(object(), mpz_class(mp));
  }

  // this should be external but js doesn't support overloading!
  Integer &sqrt() /*const*/;
  int sgn() /*const*/;
  Integer &abs() /*const*/;

  // operators
  void cmp(flusspferd::call_context &x) /*const*/;

  void add(flusspferd::call_context &x) /*const*/;
  void sub(flusspferd::call_context &x) /*const*/;
  void mul(flusspferd::call_context &x) /*const*/;
  void div(flusspferd::call_context &x) /*const*/;
};

}

#endif
