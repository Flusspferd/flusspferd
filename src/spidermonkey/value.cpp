// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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

#include "flusspferd/value.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/object.hpp"
#include "flusspferd/spidermonkey/string.hpp"
#include <js/jsapi.h>
#include <cassert>
#include <cmath>
#ifdef WIN32
#include <float.h>
#endif

using namespace flusspferd;

value::~value() { }
value::value() : Impl::value_impl(JSVAL_VOID) { }
value::value(bool b) : Impl::value_impl(BOOLEAN_TO_JSVAL(b)) { }
value::value(int i) : Impl::value_impl(INT_TO_JSVAL(i)) { }

static jsval from_double(double d) {
  jsval ret;
  if (!JS_NewNumberValue(Impl::current_context(), d, &ret))
    throw exception("Conversion from double to value failed");
  return ret;
}

value::value(double d) : Impl::value_impl(from_double(d)) { }
value::value(object const &o)
  : Impl::value_impl(OBJECT_TO_JSVAL(Impl::get_object(
      const_cast<object&>(o))))
{ }
value::value(string const &s)
  : Impl::value_impl(STRING_TO_JSVAL(Impl::get_string(
      const_cast<string&>(s))))
{ }

bool value::is_null() const { return JSVAL_IS_NULL(get()); }
bool value::is_undefined() const { return JSVAL_IS_VOID(get()); }
bool value::is_int() const { return JSVAL_IS_INT(get()); }
bool value::is_double() const { return JSVAL_IS_DOUBLE(get()); }
bool value::is_number() const { return JSVAL_IS_NUMBER(get()); }
bool value::is_boolean() const { return JSVAL_IS_BOOLEAN(get()); }
bool value::is_string() const { return JSVAL_IS_STRING(get()); }
bool value::is_object() const { return JSVAL_IS_OBJECT(get()); }
bool value::is_function() const {
  return JS_TypeOfValue(Impl::current_context(), get()) == JSTYPE_FUNCTION;
}

bool value::get_boolean() const {
  assert(is_boolean());
  return JSVAL_TO_BOOLEAN(get());
}
int value::get_int() const {
  assert(is_int());
  return JSVAL_TO_INT(get());
}
double value::get_double() const {
  assert(is_double());
  jsdouble *d = JSVAL_TO_DOUBLE(get());
  assert(d);
  return *d;
}
object value::get_object() const {
  assert(is_object());
  return Impl::wrap_object(JSVAL_TO_OBJECT(get()));
}
string value::get_string() const {
  assert(is_string());
  return Impl::wrap_string(JSVAL_TO_STRING(get()));
}

string value::to_string() const {
  return string(*this);
}

std::string value::to_std_string() const {
  return string(*this).to_string();
}

double value::to_number() const {
  double value;
  if (!JS_ValueToNumber(Impl::current_context(), get(), &value))
    throw exception("Could not convert value to number");
  return value;
}

double value::to_integral_number(int bits, bool signedness) const {
  long double value = to_number();
#ifdef WIN32
  if (!_finite(value))
    return 0;
#else
  if (!std::isfinite(value))
    return 0;
#endif
  long double maxU = powl(2, bits);
  value = value < 0 ? ceill(value) : floorl(value);
  value = fmodl(value, maxU);
  if (value < 0)
    value += maxU;
  if (signedness) {
    long double maxS = maxU / 2;
    if (value > maxS)
      value -= maxU;
  }
  return value;
}

bool value::to_boolean() const {
  JSBool result;
  if (!JS_ValueToBoolean(Impl::current_context(), get(), &result))
    throw exception("Could not convert value to boolean");
  return result != JS_FALSE;
}

object value::to_object() const {
  JSObject *result;
  if (!JS_ValueToObject(Impl::current_context(), get(), &result))
    throw exception("Could not convert value to object");
  return Impl::wrap_object(result);
}

void value::bind(value o) {
  setval(JSVAL_VOID);
  setp(o.getp());
}

void value::unbind() {
  setval(JSVAL_VOID);
  setp(getvalp());
}

