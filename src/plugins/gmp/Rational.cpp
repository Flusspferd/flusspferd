#include "Rational.hpp"

#include "exception.hpp"

#include "Float.hpp"
#include "Integer.hpp"

using namespace flusspferd;

namespace multi_precision {
  Rational::Rational(flusspferd::object const &self, mpq_class const &mp)
    : base_type(self), mp(mp)
  { }

  Rational::Rational(flusspferd::object const &self, flusspferd::call_context &x)
    : base_type(self)
  {
    try {
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
        if(v.is_int() && u.is_int()) {
          mp = mpq_class(v.get_int(), u.get_int());
        }
        else if(v.is_string() && u.is_int()) {
          if(mp.set_str(v.to_std_string(), u.get_int()) != 0) {
            throw runtime_error("string representation not valid");
          }
        }
        else {
          throw argument_error("Wrong arguments! (string, int) expected.");
        }
      }
      else {
        throw argument_error("Wrong number of arguments");
      }
    }
    catch(std::invalid_argument &e) {
      throw runtime_error(e.what());
    }
  }

  double Rational::get_double() /*const*/ {
    return mp.get_d();
  }

  void Rational::get_string(flusspferd::call_context &cc) /*const*/ {
    if(cc.arg.size() == 0) {
      cc.result = mp.get_str();
    }
    else if(cc.arg.size() == 1) {
      if(!cc.arg[0].is_int()) {
        throw type_error("gmp.Rational#toString wrong argument type");
      }
      cc.result = mp.get_str(cc.arg[0].get_int());
    }
    else {
      throw argument_error("gmp.Rational#toString wrong number of arguments");
    }
  }

  int Rational::sgn() /*const*/ {
    return ::sgn(mp);
  }
  Rational &Rational::abs() /*const*/ {
    return Rational::create_rational(::abs(mp));
  }
  void Rational::canonicalize() {
    mp.canonicalize();
  }

  Integer &Rational::get_num() /*const*/ {
    return Integer::create_integer(mp.get_num());
  }

  void Rational::set_num(Integer &i) {
    mpq_set_num(mp.get_mpq_t(), i.mp.get_mpz_t());
  }

  Integer &Rational::get_den() /*const*/ {
    return Integer::create_integer(mp.get_den());
  }

  void Rational::set_den(Integer &i) {
    mpq_set_den(mp.get_mpq_t(), i.mp.get_mpz_t());
  }

  void Rational::cmp(flusspferd::call_context &x) /*const*/ {
    if(x.arg.empty() || x.arg.size() > 1)
      throw argument_error("Expected one argument");
    flusspferd::value v = x.arg.front();
    if(v.is_int())
      x.result = ::cmp(mp, v.get_int());
    else if(v.is_double())
      x.result = ::cmp(mp, v.get_double());
    else if(v.is_object()) {
      if(flusspferd::is_native<Integer>(v.get_object()))
        x.result = ::cmp(mp, flusspferd::get_native<Integer>(v.get_object()).mp);
      else if(flusspferd::is_native<Float>(v.get_object()))
        x.result = ::cmp(mp, flusspferd::get_native<Float>(v.get_object()).mp);
      else if(flusspferd::is_native<Rational>(v.get_object()))
        x.result = ::cmp(mp, flusspferd::get_native<Rational>(v.get_object()).mp);
      else
        throw type_error("Wrong parameter type");
    }
    else
      throw type_error("Wrong parameter type");
  }

#define OPERATOR(name, op)                                              \
  void Rational:: name (flusspferd::call_context &x) /*const*/ {        \
    if(x.arg.empty() || x.arg.size() > 1)                               \
      throw argument_error("Expected one argument");                    \
    flusspferd::value v = x.arg.front();                                \
    if(v.is_int())                                                      \
      x.result = create_rational(mp op v.get_int());                    \
    else if(v.is_double())                                              \
      x.result = create_rational(mp op v.get_double());                 \
    else if(v.is_object()) {                                            \
      if(flusspferd::is_native<Integer>(v.get_object()))                \
        x.result = create_rational(mp op flusspferd::get_native<Integer>(v.get_object()).mp); \
      else if(flusspferd::is_native<Rational>(v.get_object()))          \
        x.result = create_rational(mp op flusspferd::get_native<Rational>(v.get_object()).mp); \
      else if(flusspferd::is_native<Float>(v.get_object()))             \
        x.result = create_rational(mp op flusspferd::get_native<Float>(v.get_object()).mp); \
      else                                                              \
        throw type_error("Wrong argument type");                        \
    }                                                                   \
    else                                                                \
      throw type_error("Wrong argument type");                          \
  }                                                                     \
  /**/

  OPERATOR(add, +)
  OPERATOR(sub, -)
  OPERATOR(mul, *)
  OPERATOR(div, /)

#undef OPERATOR
}
