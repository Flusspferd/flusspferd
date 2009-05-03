#include "Rational.hpp"

#include "Float.hpp"
#include "Integer.hpp"

using namespace flusspferd;

namespace multi_precission {
  Rational::Rational(flusspferd::object const &self, mpq_class const &mp)
    : flusspferd::native_object_base(self), mp(mp)
  { }

  Rational::Rational(flusspferd::object const &self, flusspferd::call_context &x)
    : flusspferd::native_object_base(self)
  {
    // TODO
    if(x.arg.size() == 1) {
      flusspferd::value v = x.arg.front();
      if(v.is_double())
        mp = v.get_double();
      else if(v.is_int())
        mp = v.get_int();
      else if(v.is_string())
        mp = v.to_std_string();
      else
        mp = flusspferd::get_native<Integer>(v.get_object()).mp;
    }
    else
      throw flusspferd::exception("Wrong number of arguments!");
  }
}
