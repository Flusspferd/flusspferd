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

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/security.hpp"

#include <gmpxx.h>

using namespace flusspferd;

namespace {

namespace multi_precission {
struct Integer : public flusspferd::native_object_base {
  struct class_info : flusspferd::class_info {
    static char const *constructor_name() {
      return "Integer";
    }

    static char const *full_name() {
      return "Integer";
    }

    static object create_prototype() {
      flusspferd::object proto = flusspferd::create_object();
      create_native_method(proto, "fits_int", &Integer::fits_int);
      create_native_method(proto, "get_int", &Integer::get_int);
      create_native_method(proto, "get_double", &Integer::get_double);
      create_native_method(proto, "get_string", &Integer::get_string);
      create_native_method(proto, "get_string_base", &Integer::get_string_base);
      create_native_method(proto, "sqrt", &Integer::sqrt);
      create_native_method(proto, "sgn", &Integer::sgn);
      create_native_method(proto, "abs", &Integer::abs);
      return proto;
    }
  };
  mpz_class mp;

  Integer(flusspferd::object const &self, mpz_class const &mp)
    : flusspferd::native_object_base(self), mp(mp)
  { }

  Integer(flusspferd::object const &self, flusspferd::call_context &x)
    : flusspferd::native_object_base(self)
  {
    if(x.arg.size() == 1) {
      value v = x.arg.front();
      if(v.is_double())
        mp = v.get_double();
      else if(v.is_int())
        mp = v.get_int();
      else if(v.is_string())
        mp = v.to_std_string();
      else
        mp = flusspferd::get_native<Integer>(v.get_object()).mp;
    }
    else if(x.arg.size() == 2) {
      value v = x.arg.front();
      value u = x.arg.back();
      if(v.is_string() && u.is_int())
        mp.set_str(v.to_std_string(), u.get_int());
      else
        throw flusspferd::exception("Wrong arguments! (string, int) expected.");
    }
    else
      throw flusspferd::exception("Wrong number of arguments!");
  }

  bool fits_int() /*const*/ {
    return mp.fits_sint_p();
  }
  int get_int() /*const*/ {
    assert(fits_int());
    return mp.get_si();
  }
  double get_double() /*const*/ {
    return mp.get_d();
  }
  std::string get_string() /*const*/ {
    return mp.get_str();
  }
  std::string get_string_base(int base) /*const*/ {
    return mp.get_str(base);
  }

  template<typename T>
  Integer &create_integer(T mp) /*const*/ {
    return create_native_object<Integer>(object(), mpz_class(mp));
  }

  // this should be external but js doesn't support overloading!
  Integer &sqrt() /*const*/ {
    return create_integer(::sqrt(mp));
  }

  int sgn() /*const*/ {
    return ::sgn(mp);
  }

  Integer &abs() /*const*/ {
    return create_integer(::abs(mp));
  }
};


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
      create_native_method(proto, "get_double", &Float::get_double);
      create_native_method(proto, "get_string", &Float::get_string);
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

  Float(flusspferd::object const &self, mpf_class const &mp)
    : flusspferd::native_object_base(self), mp(mp)
  { }

  void init_with_value(value v) {
    if(v.is_double())
      mp = v.get_double();
    else if(v.is_int())
      mp = v.get_int();
    else if(v.is_string())
      mp = v.to_std_string();
    else
      mp = flusspferd::get_native<Float>(v.get_object()).mp;
  }

  Float(flusspferd::object const &self, flusspferd::call_context &x)
    : flusspferd::native_object_base(self)
  {
    if(x.arg.size() == 1) {
      init_with_value(x.arg.front());
    }
    else if(x.arg.size() == 2) {
      value v = x.arg.front();
      value u = x.arg.back();
      if(v.is_string() && u.is_int())
        mp.set_str(v.to_std_string(), u.get_int());
      else {
        mp.set_prec(u.get_int());
        init_with_value(v);
      }
    }
    else if(x.arg.size() == 3) {
      value v = x.arg[0];
      value u = x.arg[1];
      value w = x.arg[2];
      if(v.is_string() && u.is_int() && w.is_int()) {
        mp.set_prec(u.get_int());
        mp.set_str(v.to_std_string(), w.get_int());
      }
      else
        throw flusspferd::exception("Wrong arguments! (string, int, int) expected.");
    }
    else
      throw flusspferd::exception("Wrong number of arguments!");
  }

  bool fits_int() /*const*/ {
    return mp.fits_sint_p();
  }
  int get_int() /*const*/ {
    assert(fits_int());
    return mp.get_si();
  }
  double get_double() /*const*/ {
    return mp.get_d();
  }
  std::string get_string() /*const*/ {
    mp_exp_t expo; // TODO handle expo
    return mp.get_str(expo);
  }
  std::string get_string_base(int base) /*const*/ {
    mp_exp_t expo; // TODO handle expo
    return mp.get_str(expo, base);
  }
  int get_prec() /*const*/ {
    return mp.get_prec();
  }
  void set_prec(int p) {
    mp.set_prec(p);
  }

  template<typename T>
  Float &create_float(T mp) /*const*/ {
    return create_native_object<Float>(object(), mpf_class(mp));
  }

  // this should be external but js doesn't support overloading!
  Float &sqrt() /*const*/ {
    return create_float(::sqrt(mp));
  }

  int sgn() /*const*/ {
    return ::sgn(mp);
  }

  Float &abs() /*const*/ {
    return create_float(::abs(mp));
  }

  Float &ceil() /*const*/ {
    return create_float(::ceil(mp));
  }

  Float &floor() /*const*/ {
    return create_float(::floor(mp));
  }

  // hypot

  Float &trunc() /*const*/ {
    return create_float(::trunc(mp));
  }

  // operators
  void cmp(flusspferd::call_context &x) {
    if(x.arg.empty() || x.arg.size() > 1)
      throw flusspferd::exception("Expected one parameter!");
    value v = x.arg.front();
    if(v.is_int())
      x.result = ::cmp(v.get_int(), mp);
    else if(v.is_double())
      x.result = ::cmp(v.get_double(), mp);
    else
      x.result = ::cmp(flusspferd::get_native<Float>(v.get_object()).mp, mp);
  }

  Float &add(Float const &f) { // TODO Integer
    return create_float(mp + f.mp);
  }

  Float &sub(Float const &f) { // TODO Integer
    return create_float(mp - f.mp);
  }

  Float &mul(Float const &f) { // TODO Integer
    return create_float(mp * f.mp);
  }

  Float &div(Float const &f) { // TODO Integer
    return create_float(mp / f.mp);
  }
};
}

extern "C" void flusspferd_load(object gmp) {
  load_class<multi_precission::Integer>(gmp);
  // TODO Rational
  load_class<multi_precission::Float>(gmp);
}
}
