#include "Float.hpp"

#include "Integer.hpp"
#include "Rational.hpp"

using namespace flusspferd;

namespace multi_precission {
  Float::Float(flusspferd::object const &self, mpf_class const &mp)
    : base_type(self), mp(mp)
  { }

  Float::Float(flusspferd::object const &self, flusspferd::call_context &x)
    : base_type(self)
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

  bool Float::fits_int() /*const*/ {
    return mp.fits_sint_p();
  }

  int Float::get_int() /*const*/ {
    assert(fits_int());
    return mp.get_si();
  }

  double Float::get_double() /*const*/ {
    return mp.get_d();
  }

  std::string Float::get_string() /*const*/ {
    mp_exp_t expo; // TODO handle expo
    return mp.get_str(expo);
  }

  std::string Float::get_string_base(int base) /*const*/ {
    mp_exp_t expo; // TODO handle expo
    return mp.get_str(expo, base);
  }

  int Float::get_prec() /*const*/ {
    return mp.get_prec();
  }

  void Float::set_prec(int p) {
    mp.set_prec(p);
  }

  Float &Float::sqrt() /*const*/ {
    return create_float(::sqrt(mp));
  }

  int Float::sgn() /*const*/ {
    return ::sgn(mp);
  }

  Float &Float::abs() /*const*/ {
    return create_float(::abs(mp));
  }

  Float &Float::ceil() /*const*/ {
    return create_float(::ceil(mp));
  }

  Float &Float::floor() /*const*/ {
    return create_float(::floor(mp));
  }

  Float &Float::trunc() /*const*/ {
    return create_float(::trunc(mp));
  }

  void Float::cmp(flusspferd::call_context &x) /*const*/ {
    if(x.arg.empty() || x.arg.size() > 1)
      throw flusspferd::exception("Expected one parameter");
    flusspferd::value v = x.arg.front();
    if(v.is_int())
      x.result = ::cmp(mp, v.get_int());
    else if(v.is_double())
      x.result = ::cmp(mp, v.get_double());
    else if(flusspferd::is_native<Integer>(v.get_object()))
      x.result = ::cmp(mp, flusspferd::get_native<Integer>(v.get_object()).mp);
    else if(flusspferd::is_native<Rational>(v.get_object()))
      x.result = ::cmp(mp, flusspferd::get_native<Rational>(v.get_object()).mp);
    else if(flusspferd::is_native<Float>(v.get_object()))
      x.result = ::cmp(mp, flusspferd::get_native<Float>(v.get_object()).mp);
    else
      throw flusspferd::exception("Wrong parameter type");
  }

#define OPERATOR(name, op)                                              \
  void Float:: name (flusspferd::call_context &x) /*const*/ {           \
    if(x.arg.empty() || x.arg.size() > 1)                               \
      throw flusspferd::exception("Expected on parameter");             \
    flusspferd::value v = x.arg.front();                                \
    if(v.is_int())                                                      \
      x.result = create_float(mp op v.get_int());                       \
    else if(v.is_double())                                              \
      x.result = create_float(mp op v.get_double());                    \
    else if(flusspferd::is_native<Integer>(v.get_object()))             \
      x.result = create_float(mp op flusspferd::get_native<Integer>(v.get_object()).mp); \
    else if(flusspferd::is_native<Rational>(v.get_object()))            \
      x.result = create_float(mp op flusspferd::get_native<Rational>(v.get_object()).mp); \
    else if(flusspferd::is_native<Float>(v.get_object()))               \
      x.result = create_float(mp op flusspferd::get_native<Float>(v.get_object()).mp); \
    else \
      throw flusspferd::exception("Wrong parameter type");      \
  } \
  /**/

  OPERATOR(add, +)
  OPERATOR(sub, -)
  OPERATOR(mul, *)
  OPERATOR(div, /)

#undef OPERATOR

  void Float::init_with_value(value v) {
    if(v.is_double())
      mp = v.get_double();
    else if(v.is_int())
      mp = v.get_int();
    else if(v.is_string())
      mp = v.to_std_string();
    else if(flusspferd::is_native<Integer>(v.get_object()))
      mp = flusspferd::get_native<Integer>(v.get_object()).mp;
    else if(flusspferd::is_native<Rational>(v.get_object()))
      mp = flusspferd::get_native<Rational>(v.get_object()).mp;
    else if(flusspferd::is_native<Float>(v.get_object()))
      mp = flusspferd::get_native<Float>(v.get_object()).mp;
    else
      throw flusspferd::exception("Wrong parameter type");
  }
}

