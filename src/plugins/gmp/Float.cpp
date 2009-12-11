#include "Float.hpp"

#include "exception.hpp"

#include "Integer.hpp"
#include "Rational.hpp"

using namespace flusspferd;

namespace multi_precision {
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
      if(v.is_string() && u.is_int()) {
        if(mp.set_str(v.to_std_string(), u.get_int()) != 0) {
          throw runtime_error("string representation not valid");
        }
      }
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
        if(mp.set_str(v.to_std_string(), w.get_int()) != 0) {
          throw runtime_error("string representation not valid");
        }
      }
      else
        throw argument_error("Wrong arguments! (string, int, int) expected.");
    }
    else
      throw argument_error("Wrong number of arguments!");
  }

  bool Float::fits_int() /*const*/ {
    return mp.fits_sint_p();
  }

  int Float::get_int() /*const*/ {
    if(!fits_int()) {
      throw runtime_error("gmp.Float does not fit into Javascript Number!");
    }
    return mp.get_si();
  }

  double Float::get_double() /*const*/ {
    return mp.get_d();
  }

  std::pair<std::string,mp_exp_t>
  Float::get_string_impl(flusspferd::call_context &cc) const {
    mp_exp_t expo;
    std::string str;
    if(cc.arg.size() == 0) {
      str = mp.get_str(expo);
    }
    else if(cc.arg.size() == 1) { // base parameter
      if(!cc.arg[0].is_int()) {
        throw type_error("gmp.Float#getString expects and Integer or none parameter.");
      }
      str = mp.get_str(expo, cc.arg[0].get_int());
    }
    else {
      throw argument_error("gmp.Float#getString expects one or zero parameters.");
    }
    return std::make_pair(str, expo);
  }

  void Float::toString(flusspferd::call_context &cc) /* const */ {
    std::pair<std::string,mp_exp_t> p = get_string_impl(cc);
    assert(p.second >= 0);
    std::size_t const expo = static_cast<std::size_t>(p.second);
    if(expo >= p.first.size()) {
      for(std::size_t i = p.first.size(); i < expo; ++i) {
        p.first += '0';
      }
      p.first += ".0";
      cc.result = p.first;
    }
    else {
      cc.result = p.first.substr(0, expo) + '.' + p.first.substr(expo);
    }
  }

  void Float::get_string(flusspferd::call_context &cc) /*const*/ {
    std::pair<std::string,mp_exp_t> p = get_string_impl(cc);
    object x = create<object>();
    x.set_property("string", value(p.first));
    x.set_property("exp", value(p.second));
    cc.result = x;
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
      throw argument_error("Expected one argument");
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
      throw type_error("Wrong argument type");
  }

#define OPERATOR(name, op)                                              \
  void Float:: name (flusspferd::call_context &x) /*const*/ {           \
    if(x.arg.empty() || x.arg.size() > 1)                               \
      throw argument_error("Expected one argument");                    \
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
    else                                                                \
      throw type_error("Wrong parameter type");                         \
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
      throw type_error("Wrong parameter type");
  }
}

