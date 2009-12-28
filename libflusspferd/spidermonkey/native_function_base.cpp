// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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
#include "flusspferd/tracer.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include "flusspferd/spidermonkey/function.hpp"
#include "flusspferd/current_context_scope.hpp"
#include <boost/foreach.hpp>
#include <js/jsapi.h>

using namespace flusspferd;

class native_function_base::impl {
public:
  impl()
  {}

  static JSBool call_helper(
    JSContext *ctx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
  static void finalize(JSContext *, JSObject *);

  static void trace_op(JSTracer *trc, JSObject *obj);

  static JSClass function_priv_class;
};

native_function_base::native_function_base(function const &obj)
  : p(new impl)
{
  load_into(obj);
}

native_function_base::~native_function_base() { }


#define MARK_TRACE_OP ((JSMarkOp) &native_function_base::impl::trace_op)

JSClass native_function_base::impl::function_priv_class = {
  "FunctionParent",
  JSCLASS_HAS_PRIVATE
  | JSCLASS_MARK_IS_TRACE
  ,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,
  &native_function_base::impl::finalize,
  0,
  0,
  0,
  0,
  0,
  0,
  MARK_TRACE_OP,
  0
};

#undef MARK_TRACE_OP

function native_function_base::create_function(
    unsigned arity, std::string const &name)
{
  JSContext *ctx = Impl::current_context();

  JSObject *priv = JS_NewObject(ctx, &impl::function_priv_class, 0, 0);

  root_object priv_o(Impl::wrap_object(priv));

  if (!priv)
    throw exception("Could not create native function");

  JSFunction *fun = JS_NewFunction(
      ctx, &impl::call_helper,
      arity, 0, 0, name.c_str());

  if (!fun)
    throw exception("Could not create native function");

  function funx(Impl::wrap_function(fun));

  JSObject *obj = Impl::get_object(funx);

  JS_SetReservedSlot(ctx, obj, 0, OBJECT_TO_JSVAL(priv));

  return funx;
}

void native_function_base::load_into(function const &fun) {
  if (fun.is_null())
    throw exception("Cannot initalise native_function_base "
                    "with a null function");

  JSContext *ctx = Impl::current_context();

  function::operator=(fun);

  JSObject *obj = Impl::get_object(*this);

  value priv_v;
  if (!JS_GetReservedSlot(ctx, obj, 0, Impl::get_jsvalp(priv_v)))
    throw exception("Could not initialise native_function_base (internal error");

  JSObject *priv = Impl::get_object(priv_v.get_object());

  JS_SetPrivate(ctx, priv, this);

  JS_SetReservedSlot(ctx, obj, 1, PRIVATE_TO_JSVAL(this));
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


void native_function_base::impl::trace_op(
    JSTracer *trc, JSObject *p)
{
  // NOTE: avoid any rooting

  JSContext *ctx = trc->context;

  native_function_base *self = 0;

  self = (native_function_base *) JS_GetInstancePrivate(ctx, p, &impl::function_priv_class, 0);

  if (!self) {
    if (JS_ObjectIsFunction(ctx, p)) {
      jsval p_val;

      if (JS_GetReservedSlot(ctx, p, 0, &p_val)) {
        p = JSVAL_TO_OBJECT(p_val);

        if (p)
          self =
            (native_function_base *) JS_GetInstancePrivate(ctx, p, &impl::function_priv_class, 0);
      }
    }
  }

  if (self) {
    current_context_scope scope(Impl::wrap_context(ctx));
    tracer tracer_(trc);
    self->trace(tracer_);
  }
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

  root_object o(o_);
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

void native_function_base::trace(tracer&) {}
