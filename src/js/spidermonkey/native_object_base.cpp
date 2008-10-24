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

#include "flusspferd/native_object_base.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/call_context.hpp"
#include "flusspferd/root_value.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/function.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include <boost/unordered_map.hpp>

using namespace flusspferd;

class native_object_base::impl {
public:
  static void finalize(JSContext *ctx, JSObject *obj);
  static JSBool call_helper(JSContext *, JSObject *, uintN, jsval *, jsval *);
  static uint32 mark_op(JSContext *, JSObject *, void *);

  static JSClass native_object_class;

public:
  typedef boost::unordered_map<std::string, native_method_type> native_method_map;

  native_method_map native_methods;
};

JSClass native_object_base::impl::native_object_class = {
    "NativeObject",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, &native_object_base::impl::finalize,
    0, 0,
    &native_object_base::impl::call_helper,
    0, 0, 0,
    &native_object_base::impl::mark_op,
    0
  };

native_object_base::native_object_base() : p(new impl) {
  add_native_method("()", 0, &native_object_base::invalid_method);
}

native_object_base::~native_object_base() {}

void native_object_base::post_initialize() {}

void native_object_base::call_native_method(std::string const &name, call_context &x) {
  impl::native_method_map::iterator it = p->native_methods.find(name);

  if (it != p->native_methods.end()) {
    native_method_type m = it->second;
    if (m)
      (this->*m)(x);
  }
}

void native_object_base::invalid_method(call_context &) {
  throw exception("Invalid method");
}

void native_object_base::trace(tracer&) {}

native_object_base *native_object_base::get_native(object const &o_) {
  object o = o_;

  JSContext *ctx = Impl::current_context();

  void *priv = JS_GetInstancePrivate(ctx, Impl::get_object(o), &impl::native_object_class, 0);

  if (!priv)
    throw exception("Object is not native");

  return static_cast<native_object_base*>(priv);
}

object native_object_base::create_object() {
  JSContext *ctx = Impl::current_context();

  JSObject *o = JS_NewObject(ctx, &impl::native_object_class, 0, 0);

  if (!o)
    throw exception("Could not create native object");

  object::operator=(Impl::wrap_object(o));
  root_value r(get_object());

  if (!JS_SetPrivate(ctx, o, this))
    throw exception("Could not create native object (private data)");

  post_initialize();

  return *this;
}

void native_object_base::add_native_method(std::string const &name, unsigned arity) {
  JSContext *ctx = Impl::current_context();

  JSFunction *func = JS_DefineFunction(
      ctx,
      this->object::get(),
      name.c_str(),
      &impl::call_helper,
      arity,
      0);

  if (!func)
    throw exception("Could not create native method " + name); 
}

void native_object_base::add_native_method(
  std::string const &name, unsigned arity, native_method_type method)
{
  p->native_methods[name] = method;
  if (name != "()")
    add_native_method(name, arity);
}

void native_object_base::impl::finalize(JSContext *ctx, JSObject *obj) {
  current_context_scope scope(Impl::wrap_context(ctx));
  delete native_object_base::get_native(Impl::wrap_object(obj));
}

JSBool native_object_base::impl::call_helper(
    JSContext *ctx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  FLUSSPFERD_CALLBACK_BEGIN {
    current_context_scope scope(Impl::wrap_context(ctx));

    JSObject *function = JSVAL_TO_OBJECT(argv[-2]);

    native_object_base *self = 0;
    
    try {
      self = native_object_base::get_native(Impl::wrap_object(obj));
    } catch (exception &) {
      self = native_object_base::get_native(Impl::wrap_object(function));
    }

    call_context x;

    x.self = Impl::wrap_object(obj);
    x.arg = Impl::arguments_impl(argc, argv);
    x.result.bind(Impl::wrap_jsvalp(rval));
    x.function = Impl::wrap_object(function);

    std::string name = "()";

    try {
      name = flusspferd::function(x.function).name().to_string();
    } catch (exception&) {}

    self->call_native_method(name, x);
  } FLUSSPFERD_CALLBACK_END;
}

uint32 native_object_base::impl::mark_op(JSContext *ctx, JSObject *obj, void *thing) {
  // TODO: callback?

  current_context_scope scope(Impl::wrap_context(ctx));

  native_object_base *self = native_object_base::get_native(Impl::wrap_object(obj));

  tracer trc(thing);
  self->trace(trc);

  return 0;
}
