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

#ifndef FLUSSPFERD_PLUGINS_GMP_FLOAT_HPP
#define FLUSSPFERD_PLUGINS_GMP_FLOAT_HPP

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/class_description.hpp"
#include <gmpxx.h>

namespace multi_precision {

FLUSSPFERD_CLASS_DESCRIPTION(
  Float,
  (constructor_name, "Float")
  (full_name, "gmp.Float")
  (methods,
    ("fits_int",         bind,    fits_int)
    ("fitsInt",          alias,   "fits_int")
    ("get_int",          bind,    get_int)
    ("toInt",            alias,   "get_int")
    ("get_double",       bind,    get_double)
    ("toDouble",         alias,   "get_double")
    ("get_string",       bind,    get_string)
    ("getString",        alias,   "get_string")
    ("toString",         bind,    toString)
    ("get_string_base",  alias,   "get_string") // deprecated
    ("get_prec",         bind,    get_prec)
    ("set_prec",         bind,    set_prec)
    ("sqrt",             bind,    sqrt)
    ("sgn",              bind,    sgn)
    ("abs",              bind,    abs)
    ("ceil",             bind,    ceil)
    ("floor",            bind,    floor)
    ("trunc",            bind,    trunc)
    ("cmp",              bind,    cmp)
    ("add",              bind,    add)
    ("sub",              bind,    sub)
    ("mul",              bind,    mul)
    ("div",              bind,    div))
  (properties,
   ("precision", getter_setter, (get_prec, set_prec))))
{
public:
  mpf_class mp;

  Float(flusspferd::object const &self, mpf_class const &mp);
  Float(flusspferd::object const &self, flusspferd::call_context &x);

  bool fits_int() /*const*/;
  int get_int() /*const*/;
  double get_double() /*const*/;
  std::string toString() /* const */;
  void get_string(flusspferd::call_context &x) /*const*/;
  int get_prec() /*const*/;
  void set_prec(int p);

  template<typename T>
  Float &create_float(T mp) const {
    return flusspferd::create<Float>(boost::fusion::vector1<mpf_class>(mp));
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
