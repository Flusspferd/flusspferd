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

#include "flusspferd/create.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/native_function_base.hpp"
#include "flusspferd/spidermonkey/object.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include <js/jsapi.h>

using namespace flusspferd;

object flusspferd::create_object() {
  JSObject *o = JS_NewObject(Impl::current_context(), 0, 0, 0);
  if (!o)
    throw exception("Could not create object");

  return Impl::wrap_object(o);
}

object flusspferd::create_array(unsigned length) {
  JSObject *o = JS_NewArrayObject(Impl::current_context(), length, 0);
  if (!o)
    throw exception("Could not create array");

  return Impl::wrap_object(o);
}

object flusspferd::create_native_object(native_object_base *ptr) {
  try {
    return ptr->create_object();
  } catch (...) {
    delete ptr;
    throw;
  }
}

function flusspferd::create_native_function(native_function_base *ptr) {
  try {
    return ptr->create_function();
  } catch (...) {
    delete ptr;
    throw;
  }
}
