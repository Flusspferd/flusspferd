#ifndef FLUSSPFERD_PLUGIN_GMP_FLOAT_IMPL_HPP
#define FLUSSPFERD_PLUGIN_GMP_FLOAT_IMPL_HPP

#include "Float.hpp"

using namespace flusspferd;

namespace multi_precission {
  Float::Float(flusspferd::object const &self, mpf_class const &mp)
    : flusspferd::native_object_base(self), mp(mp)
  { }

  Float::Float(flusspferd::object const &self, flusspferd::call_context &x)
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

  void Float::init_with_value(value v) {
    if(v.is_double())
      mp = v.get_double();
    else if(v.is_int())
      mp = v.get_int();
    else if(v.is_string())
      mp = v.to_std_string();
    else if(flusspferd::is_native<Integer>(v.get_object()))
      mp = flusspferd::get_native<Integer>(v.get_object()).mp;
    else if(flusspferd::is_native<Float>(v.get_object()))
      mp = flusspferd::get_native<Float>(v.get_object()).mp;
    else
      throw flusspferd::exception("Wrong parameter type");
  }
}

#endif
