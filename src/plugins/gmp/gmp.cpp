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
        mp = v.to_number();
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
  object create_integer(T mp) /*const*/ {
    return create_native_object<Integer>(create_object(), mpz_class(mp));
  }

  // this should be external but js doesn't support overloading!
  object sqrt() /*const*/ {
    return create_integer(::sqrt(mp));
  }

  int sgn() /*const*/ {
    return ::sgn(mp);
  }

  object abs() /*const*/ {
    return create_integer(::abs(mp));
  }
};

  // ...
}

extern "C" void flusspferd_load(object gmp) {
  load_class<multi_precission::Integer>(gmp);
}
}
