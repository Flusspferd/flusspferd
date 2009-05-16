// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#include "flusspferd/array.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include <js/jsapi.h>

using namespace flusspferd;

array::array(object const &o) : object(o) {
  check();
}

array &array::operator=(object const &o) {
  object::operator=(o);
  check();
  return *this;
}

void array::check() {
  if (!JS_IsArrayObject(Impl::current_context(), get()))
    throw exception("Object is not array");
}

std::size_t array::length() const {
  jsuint length;
  if (!JS_GetArrayLength(Impl::current_context(), get_const(), &length))
    throw exception("Could not get array length");
  return length;
}

void array::set_length(std::size_t length) {
  if (!JS_SetArrayLength(Impl::current_context(), get(), length))
    throw exception("Could not set array length");
}

value array::get_element(std::size_t index) const {
  value result;
  if (!JS_GetElement(Impl::current_context(),
                     get_const(), index,
                     Impl::get_jsvalp(result)))
    throw exception("Could not get array element");
  return result;
}

void array::set_element(std::size_t index, value const &v_) {
  value v(v_);
  if (!JS_SetElement(Impl::current_context(),
                     get(), index, Impl::get_jsvalp(v)))
    throw exception("Could not set array element");
}
