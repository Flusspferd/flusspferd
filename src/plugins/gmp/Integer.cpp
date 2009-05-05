
#include "Integer.hpp"

#include "Float.hpp"
#include "Rational.hpp"

namespace multi_precission {
  Integer::Integer(flusspferd::object const &self, mpz_class const &mp)
    : flusspferd::native_object_base(self), mp(mp)
  { }

  Integer::Integer(flusspferd::object const &self, flusspferd::call_context &x)
    : flusspferd::native_object_base(self)
  {
    if(x.arg.size() == 1) {
      flusspferd::value v = x.arg.front();
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
    else if(x.arg.size() == 2) {
      flusspferd::value v = x.arg.front();
      flusspferd::value u = x.arg.back();
      if(v.is_string() && u.is_int())
        mp.set_str(v.to_std_string(), u.get_int());
      else
        throw flusspferd::exception("Wrong arguments! (string, int) expected.");
    }
    else
      throw flusspferd::exception("Wrong number of arguments!");
  }

  bool Integer::fits_int() /*const*/ {
    return mp.fits_sint_p();
  }
  int Integer::get_int() /*const*/ {
    assert(fits_int());
    return mp.get_si();
  }
  double Integer::get_double() /*const*/ {
    return mp.get_d();
  }
  std::string Integer::get_string() /*const*/ {
    return mp.get_str();
  }
  std::string Integer::get_string_base(int base) /*const*/ {
    return mp.get_str(base);
  }

  Integer &Integer::sqrt() /*const*/ {
    return create_integer(::sqrt(mp));
  }

  int Integer::sgn() /*const*/ {
    return ::sgn(mp);
  }

  Integer &Integer::abs() /*const*/ {
    return create_integer(::abs(mp));
  }

  void Integer::cmp(flusspferd::call_context &x) /*const*/ {
    if(x.arg.empty() || x.arg.size() > 1)
      throw flusspferd::exception("Expected one parameter!");
    flusspferd::value v = x.arg.front();
    if(v.is_int())
      x.result = ::cmp(mp, v.get_int());
    else if(v.is_double())
      x.result = ::cmp(mp, v.get_double());
    else if(flusspferd::is_native<Rational>(v.get_object()))
      x.result = ::cmp(mp, flusspferd::get_native<Rational>(v.get_object()).mp);
    else if(flusspferd::is_native<Float>(v.get_object()))
      x.result = ::cmp(mp, flusspferd::get_native<Float>(v.get_object()).mp);
    else if(flusspferd::is_native<Integer>(v.get_object()))
      x.result = ::cmp(mp, flusspferd::get_native<Integer>(v.get_object()).mp);
    else
      throw flusspferd::exception("Wrong parameter type");
  }

#define OPERATOR(name, op)                                              \
  void Integer:: name (flusspferd::call_context &x) /*const*/ {         \
    if(x.arg.empty() || x.arg.size() > 1)                               \
      throw flusspferd::exception("Expected on parameter");             \
    flusspferd::value v = x.arg.front();                                \
    if(v.is_int())                                                      \
      x.result = create_integer(mp op v.get_int());                     \
    else if(v.is_double())                                              \
      x.result = create_integer(mp op v.get_double());                  \
    else if(flusspferd::is_native<Float>(v.get_object()))               \
      x.result = create_integer(mp op flusspferd::get_native<Float>(v.get_object()).mp); \
    else if(flusspferd::is_native<Rational>(v.get_object()))            \
      x.result = create_integer(mp op flusspferd::get_native<Rational>(v.get_object()).mp); \
    else if(flusspferd::is_native<Integer>(v.get_object()))             \
      x.result = create_integer(mp op flusspferd::get_native<Integer>(v.get_object()).mp); \
    else                                                                \
      throw flusspferd::exception("Wrong parameter type");              \
  }                                                                     \
  /* */

  OPERATOR(add, +)
  OPERATOR(sub, -)
  OPERATOR(mul, *)
  OPERATOR(div, /)

#undef OPERATOR
}
