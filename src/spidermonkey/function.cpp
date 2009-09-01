// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

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

#include "flusspferd/function.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/native_function_base.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/object.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include "flusspferd/current_context_scope.hpp"
#include <boost/scoped_array.hpp>
#include <js/jsapi.h>

using namespace flusspferd;

function::function() { }

namespace {
  Impl::function_impl get_function(Impl::object_impl o_) {
    JSContext *ctx = Impl::current_context();
    JSObject *o = Impl::get_object(o_);

    JSFunction *f = JS_ValueToFunction(ctx, OBJECT_TO_JSVAL(o));

    if (!f)
      throw exception("Could not convert object to function");

    return Impl::wrap_function(f);
  }
}

function::function(object const &o)
  : Impl::function_impl(get_function(o)),
    object(Impl::function_impl::get_object())
{ }

function::function(Impl::object_impl const &o)
  : Impl::function_impl(get_function(o)),
    object(Impl::function_impl::get_object())
{ }

std::size_t function::arity() const {
  if (is_null())
    throw exception("Could not get function arity: object is null");
  return JS_GetFunctionArity(Impl::function_impl::get_const());
}

string function::name() const {
  if (is_null())
    throw exception("Could not get function name: object is null");
  return Impl::wrap_string(
      JS_GetFunctionId(Impl::function_impl::get_const()));
}

object Impl::function_impl::get_object() {
  JSFunction *fn = get();
  if (!fn)
    return object();
  JSObject *o = JS_GetFunctionObject(fn);
  return Impl::wrap_object(o);
}
