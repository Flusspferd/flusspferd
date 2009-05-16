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

#include "flusspferd/native_function_base.hpp"
#include "flusspferd/call_context.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include "flusspferd/spidermonkey/function.hpp"
#include "flusspferd/current_context_scope.hpp"
#include <boost/foreach.hpp>
#include <js/jsapi.h>

using namespace flusspferd;

class native_function_base::impl {
public:
  impl(unsigned arity, std::string const &name)
  : arity(arity), name(name)
  {}

  static JSBool call_helper(
    JSContext *ctx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
  static void finalize(JSContext *, JSObject *);

  unsigned arity;
  std::string name;

  static JSClass function_priv_class;
};

native_function_base::native_function_base(unsigned arity)
: p(new impl(arity, std::string()))
{}

native_function_base::native_function_base(
  unsigned arity,
  std::string const &name
)
  : p(new impl(arity, name))
{}

native_function_base::~native_function_base() { }

JSClass native_function_base::impl::function_priv_class = {
  "FunctionParent",
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,
  &native_function_base::impl::finalize,
  JSCLASS_NO_OPTIONAL_MEMBERS
};

function native_function_base::create_function() {
  JSContext *ctx = Impl::current_context();

  JSFunction *fun;

  {
    local_root_scope scope;

    JSObject *priv = JS_NewObject(ctx, &impl::function_priv_class, 0, 0);

    if (!priv)
      throw exception("Could not create native function");

    JS_SetPrivate(ctx, priv, this);

    fun = JS_NewFunction(
        ctx, &impl::call_helper,
        p->arity, 0, 0, p->name.c_str());

    if (!fun)
      throw exception("Could not create native function");

    function::operator=(Impl::wrap_function(fun));

    JSObject *obj = Impl::get_object(*this);

    JS_SetReservedSlot(ctx, obj, 0, OBJECT_TO_JSVAL(priv));
    JS_SetReservedSlot(ctx, obj, 1, PRIVATE_TO_JSVAL(this));
  }

  return *static_cast<function *>(this);
}

JSBool native_function_base::impl::call_helper(
    JSContext *ctx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  FLUSSPFERD_CALLBACK_BEGIN {
    current_context_scope scope(Impl::wrap_context(ctx));

    JSObject *function = JSVAL_TO_OBJECT(argv[-2]);

    jsval self_val;

    if (!JS_GetReservedSlot(ctx, function, 1, &self_val))
      throw exception("Could not call native function");

    native_function_base *self = 
      (native_function_base *) JSVAL_TO_PRIVATE(self_val);

    if (!self)
      throw exception("Could not call native function");

    call_context x;

    x.self = Impl::wrap_object(obj);
    x.arg = Impl::arguments_impl(argc, argv);
    x.result.bind(Impl::wrap_jsvalp(rval));
    x.function = Impl::wrap_object(function);

    self->call(x);
  } FLUSSPFERD_CALLBACK_END;
}

void native_function_base::impl::finalize(JSContext *ctx, JSObject *priv) {
  current_context_scope scope(Impl::wrap_context(ctx));

  native_function_base *self =
    (native_function_base *) JS_GetInstancePrivate(ctx, priv, &function_priv_class, 0);

  if (!self)
    throw exception("Could not finalize native function");

  delete self;
}

native_function_base *native_function_base::get_native(object const &o_) {
  JSContext *ctx = Impl::current_context();

  object o = o_;
  JSObject *p = Impl::get_object(o);

  native_function_base *self =
    (native_function_base *) JS_GetInstancePrivate(ctx, p, &impl::function_priv_class, 0);

  if (self)
    return self;

  if (!JS_ObjectIsFunction(ctx, p))
    throw exception("Could not get native function pointer (no function)");

  jsval p_val;

  if (!JS_GetReservedSlot(ctx, p, 0, &p_val))
    throw exception("Could not get native function pointer");

  p = JSVAL_TO_OBJECT(p_val);

  if (!p)
    throw exception("Could not get native function pointer");

  self =
    (native_function_base *) JS_GetInstancePrivate(ctx, p, &impl::function_priv_class, 0);

  if (!self)
    throw exception("Could not get native function pointer");

  return self;
}
