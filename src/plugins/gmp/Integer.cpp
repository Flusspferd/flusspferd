
#include "Integer.hpp"

#include "exception.hpp"

#include "Float.hpp"
#include "Rational.hpp"

namespace multi_precision {
  Integer::Integer(flusspferd::object const &self, mpz_class const &mp)
    : base_type(self), mp(mp)
  { }

  Integer::Integer(flusspferd::object const &self, flusspferd::call_context &x)
    : base_type(self)
  {
    if(x.arg.size() == 1) {
      flusspferd::value v = x.arg.front();
      if(v.is_double())
        mp = v.get_double();
      else if(v.is_int())
        mp = v.get_int();
      else if(v.is_string())
        mp = v.to_std_string();
      else if(v.is_object()) {
        if(flusspferd::is_native<Integer>(v.get_object()))
          mp = flusspferd::get_native<Integer>(v.get_object()).mp;
        else if(flusspferd::is_native<Rational>(v.get_object()))
          mp = flusspferd::get_native<Rational>(v.get_object()).mp;
        else if(flusspferd::is_native<Float>(v.get_object()))
          mp = flusspferd::get_native<Float>(v.get_object()).mp;
        else
        throw type_error("Wrong parameter type");
      }
      else
        throw type_error("Wrong parameter type");
    }
    else if(x.arg.size() == 2) {
      flusspferd::value v = x.arg.front();
      flusspferd::value u = x.arg.back();
      if(v.is_string() && u.is_int()) {
        if(mp.set_str(v.to_std_string(), u.get_int()) == -1) {
          throw runtime_error("string representation not valid");
        }
      }
      else {
        throw type_error("Wrong arguments! (string, int) expected.");
      }
    }
    else {
      throw argument_error("Wrong number of arguments!");
    }
  }

  bool Integer::fits_int() /*const*/ {
    return mp.fits_sint_p();
  }
  int Integer::get_int() /*const*/ {
    if(!fits_int()) {
      throw runtime_error("gmp.Integer does not fit into Javascript Number!");
    }
    return mp.get_si();
  }
  double Integer::get_double() /*const*/ {
    return mp.get_d();
  }
  void Integer::get_string(flusspferd::call_context &cc) /*const*/ {
    if(cc.arg.size() == 0) {
      cc.result = mp.get_str();
    }
    else if(cc.arg.size() == 1) {
      if(!cc.arg[0].is_int()) {
        throw type_error("gmp.Integer#toString wrong parameter type");
      }
      cc.result = mp.get_str(cc.arg[0].get_int());
    }
    else {
      throw argument_error("gmp.Integer#toString wrong number of parameters");
    }
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
      throw argument_error("Expected one parameter");
    flusspferd::value v = x.arg.front();
    if(v.is_int())
      x.result = ::cmp(mp, v.get_int());
    else if(v.is_double())
      x.result = ::cmp(mp, v.get_double());
    else if(v.is_object()) {
      if(flusspferd::is_native<Rational>(v.get_object()))
        x.result = ::cmp(mp, flusspferd::get_native<Rational>(v.get_object()).mp);
      else if(flusspferd::is_native<Float>(v.get_object()))
        x.result = ::cmp(mp, flusspferd::get_native<Float>(v.get_object()).mp);
      else if(flusspferd::is_native<Integer>(v.get_object()))
        x.result = ::cmp(mp, flusspferd::get_native<Integer>(v.get_object()).mp);
      else
        throw type_error("Wrong parameter type");
    }
    else
      throw type_error("Wrong parameter type");
  }

#define OPERATOR(name, op)                                              \
  void Integer:: name (flusspferd::call_context &x) /*const*/ {         \
    if(x.arg.empty() || x.arg.size() > 1)                               \
      throw argument_error("Expected one parameter");                   \
    flusspferd::value v = x.arg.front();                                \
    if(v.is_int())                                                      \
      x.result = create_integer(mp op v.get_int());                     \
    else if(v.is_double())                                              \
      x.result = create_integer(mp op v.get_double());                  \
    else if(v.is_object()) {                                            \
      if(flusspferd::is_native<Float>(v.get_object()))                  \
        x.result = create_integer(mp op flusspferd::get_native<Float>(v.get_object()).mp); \
      else if(flusspferd::is_native<Rational>(v.get_object()))          \
        x.result = create_integer(mp op flusspferd::get_native<Rational>(v.get_object()).mp); \
      else if(flusspferd::is_native<Integer>(v.get_object()))           \
        x.result = create_integer(mp op flusspferd::get_native<Integer>(v.get_object()).mp); \
      else                                                              \
        throw type_error("Wrong parameter type");                       \
    }                                                                   \
    else                                                                \
      throw type_error("Wrong parameter type");                         \
  }                                                                     \
  /* */

  OPERATOR(add, +)
  OPERATOR(sub, -)
  OPERATOR(mul, *)
  OPERATOR(div, /)

#undef OPERATOR
}
