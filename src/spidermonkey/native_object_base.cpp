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
#include "flusspferd/root.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/function.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/implementation/init.hpp"
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

using namespace flusspferd;

class native_object_base::impl {
public:
  static void finalize(JSContext *ctx, JSObject *obj);
  static JSBool call_helper(JSContext *, JSObject *, uintN, jsval *, jsval *);

#if JS_VERSION >= 180
  static void trace_op(JSTracer *trc, JSObject *obj);
#else
  static uint32 mark_op(JSContext *, JSObject *, void *);
#endif

  template<property_mode>
  static JSBool property_op(JSContext *, JSObject *, jsval, jsval *);

  static JSBool new_resolve(JSContext *, JSObject *, jsval, uintN, JSObject **);

  static JSBool new_enumerate(JSContext *cx, JSObject *obj,
    JSIterateOp enum_op, jsval *statep, jsid *idp);

public:
  static JSClass native_object_class;
  static JSClass native_enumerable_object_class;

public:
  typedef boost::variant<native_method_type, callback_type> method_variant;

  typedef boost::unordered_map<std::string, method_variant> native_method_map;

  native_method_map native_methods;

public:
  typedef 
    boost::unordered_map<std::string, property_callback> property_callback_map;

  property_callback_map property_callbacks;
};

static const unsigned int basic_flags =
  JSCLASS_HAS_PRIVATE
  | JSCLASS_NEW_RESOLVE
#if JS_VERSION >= 180
  | JSCLASS_MARK_IS_TRACE
#endif
  ;

#if JS_VERSION >= 180
#define MARK_TRACE_OP ((JSMarkOp) &native_object_base::impl::trace_op)
#else
#define MARK_TRACE_OP (&native_object_base::impl::mark_op)
#endif

JSClass native_object_base::impl::native_object_class = {
  "NativeObject",
  basic_flags,
  &native_object_base::impl::property_op<native_object_base::property_add>,
  &native_object_base::impl::property_op<native_object_base::property_delete>,
  &native_object_base::impl::property_op<native_object_base::property_get>,
  &native_object_base::impl::property_op<native_object_base::property_set>,
  JS_EnumerateStub,
  (JSResolveOp) &native_object_base::impl::new_resolve,
  JS_ConvertStub,
  &native_object_base::impl::finalize,
  0,
  0,
  &native_object_base::impl::call_helper,
  0,
  0,
  0,
  MARK_TRACE_OP,
  0
};

JSClass native_object_base::impl::native_enumerable_object_class = {
  "NativeObject",
  basic_flags | JSCLASS_NEW_ENUMERATE,
  &native_object_base::impl::property_op<native_object_base::property_add>,
  &native_object_base::impl::property_op<native_object_base::property_delete>,
  &native_object_base::impl::property_op<native_object_base::property_get>,
  &native_object_base::impl::property_op<native_object_base::property_set>,
  (JSEnumerateOp) &native_object_base::impl::new_enumerate,
  (JSResolveOp) &native_object_base::impl::new_resolve,
  JS_ConvertStub,
  &native_object_base::impl::finalize,
  0,
  0,
  &native_object_base::impl::call_helper,
  0,
  0,
  0,
  MARK_TRACE_OP,
  0
};

native_object_base::native_object_base(object const &o) : p(new impl) {
  load_into(o);
}

native_object_base::~native_object_base() {
  if (!is_null()) {
    JS_SetPrivate(Impl::current_context(), get(), 0);
  }
}

void native_object_base::load_into(object const &o) {
  if (!is_null())
    throw exception("Cannot load native_object data into more than one object");

  object::operator=(o);

  if (!is_null()) {
    if (!JS_SetPrivate(Impl::current_context(), Impl::get_object(o), this))
      throw exception("Could not create native object (private data)");
  }
}

void native_object_base::invalid_method(call_context &) {
  throw exception("Invalid method");
}

native_object_base &native_object_base::get_native(object const &o_) {
  object o = o_;

  if (o.is_null())
    throw exception("Can not interpret 'null' as native object");

  JSContext *ctx = Impl::current_context();
  JSObject *jso = Impl::get_object(o);
  JSClass *classp = JS_GET_CLASS(ctx, jso);

  if (!classp || classp->finalize != &native_object_base::impl::finalize)
    throw exception("Object is not native");

  void *priv = JS_GetPrivate(ctx, jso);
  if (!priv)
    throw exception("Object is not native");

  return *static_cast<native_object_base*>(priv);
}

object native_object_base::do_create_object(object const &prototype_) {
  JSContext *ctx = Impl::current_context();

  object prototype = prototype_;

  JSObject *o = JS_NewObject(
      ctx,
      &impl::native_object_class,
      Impl::get_object(prototype),
      0);

  if (!o)
    throw exception("Could not create native object");

  return Impl::wrap_object(o);
}

object native_object_base::do_create_enumerable_object(object const &prototype_) {
  JSContext *ctx = Impl::current_context();

  object prototype = prototype_;

  JSObject *o = JS_NewObject(
      ctx,
      &impl::native_enumerable_object_class,
      Impl::get_object(prototype),
      0);

  if (!o)
    throw exception("Could not create native object");

  return Impl::wrap_object(o);
}

void native_object_base::add_native_method(
    std::string const &name, unsigned arity)
{
  JSContext *ctx = Impl::current_context();

  JSFunction *func = JS_DefineFunction(
      ctx,
      this->object::get(),
      name.c_str(),
      &impl::call_helper,
      arity,
      0);

  if (!func)
    throw exception(("Could not create native method " + name).c_str());
}

function native_object_base::create_native_method(
    std::string const &name, unsigned arity)
{
  JSContext *ctx = Impl::current_context();

  JSFunction *func = JS_NewFunction(
      ctx,
      &impl::call_helper,
      arity,
      0, // flags
      0, // parent
      name.c_str());

  if (!func)
    throw exception(("Could not create native method " + name).c_str());

  return Impl::wrap_function(func);
}

function native_object_base::create_native_method(
    object &container, std::string const &name, unsigned arity)
{
  function fun = create_native_method(name, arity);
  container.define_property(
      name,
      fun,
      object::dont_enumerate);
  return fun;
}

void native_object_base::register_native_method(
  std::string const &name, native_method_type method)
{
  p->native_methods[name] = method;
}

void native_object_base::register_native_method_cb(
  std::string const &name, callback_type const &cb)
{
  p->native_methods[name] = cb;
}

void native_object_base::add_property_op(
  std::string const &name, property_callback callback)
{
  p->property_callbacks[name] = callback;
}

void native_object_base::impl::finalize(JSContext *ctx, JSObject *obj) {
  void *p = JS_GetPrivate(ctx, obj);

  if (p) {
    current_context_scope scope(Impl::wrap_context(ctx));
    delete static_cast<native_object_base*>(p);
  }
}

JSBool native_object_base::impl::call_helper(
    JSContext *ctx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  FLUSSPFERD_CALLBACK_BEGIN {
    current_context_scope scope(Impl::wrap_context(ctx));

    JSObject *function = JSVAL_TO_OBJECT(argv[-2]);

    native_object_base *self = 0;
    
    try {
      self = &native_object_base::get_native(Impl::wrap_object(obj));
    } catch (exception &) {
      self = &native_object_base::get_native(Impl::wrap_object(function));
    }

    call_context x;

    x.self = Impl::wrap_object(obj);
    x.self_native = self;
    x.arg = Impl::arguments_impl(argc, argv);
    x.result.bind(Impl::wrap_jsvalp(rval));
    x.function = Impl::wrap_object(function);

    std::string name;

    try {
      name = flusspferd::function(x.function).name().to_string();
    } catch (exception&) {}

    self->call_native_method(name, x);
  } FLUSSPFERD_CALLBACK_END;
}

template<native_object_base::property_mode mode>
JSBool native_object_base::impl::property_op(
    JSContext *ctx, JSObject *obj, jsval id, jsval *vp)
{
  FLUSSPFERD_CALLBACK_BEGIN {
    current_context_scope scope(Impl::wrap_context(ctx));

    native_object_base &self =
      native_object_base::get_native(Impl::wrap_object(obj));

    value data(Impl::wrap_jsvalp(vp));
    self.property_op(mode, Impl::wrap_jsval(id), data);
  } FLUSSPFERD_CALLBACK_END;
}

JSBool native_object_base::impl::new_resolve(
    JSContext *ctx, JSObject *obj, jsval id, uintN sm_flags, JSObject **objp)
{
  FLUSSPFERD_CALLBACK_BEGIN {
    current_context_scope scope(Impl::wrap_context(ctx));

    native_object_base &self =
      native_object_base::get_native(Impl::wrap_object(obj));

    unsigned flags = 0;

    if (sm_flags & JSRESOLVE_QUALIFIED)
      flags |= property_qualified;
    if (sm_flags & JSRESOLVE_ASSIGNING)
      flags |= property_assigning;
    if (sm_flags & JSRESOLVE_DETECTING)
      flags |= property_detecting;
    if (sm_flags & JSRESOLVE_DECLARING)
      flags |= property_declaring;
    if (sm_flags & JSRESOLVE_CLASSNAME)
      flags |= property_classname;

    *objp = 0;
    if (self.property_resolve(Impl::wrap_jsval(id), flags))
      *objp = Impl::get_object(self);
  } FLUSSPFERD_CALLBACK_END;
}

JSBool native_object_base::impl::new_enumerate(
    JSContext *ctx, JSObject *obj, JSIterateOp enum_op, jsval *statep, jsid *idp)
{
  FLUSSPFERD_CALLBACK_BEGIN {
    current_context_scope scope(Impl::wrap_context(ctx));

    native_object_base &self =
      native_object_base::get_native(Impl::wrap_object(obj));

    
    boost::any *iter;
    switch (enum_op) {
    case JSENUMERATE_INIT:
      iter = new boost::any;
      int num;
      *iter = self.enumerate_start(num);
      *statep = PRIVATE_TO_JSVAL(iter);
      if (idp)
        *idp = INT_TO_JSVAL(num);
      return JS_TRUE;
    case JSENUMERATE_NEXT:
    {
      iter = (boost::any*)JSVAL_TO_PRIVATE(*statep);
      value id;
      if (iter->empty() || (id = self.enumerate_next(*iter)).is_undefined())
        *statep = JSVAL_NULL;
      else {
        JS_ValueToId(ctx, Impl::get_jsval(id), idp);
      }
      return JS_TRUE;
    }
    case JSENUMERATE_DESTROY:
      iter = (boost::any*)JSVAL_TO_PRIVATE(*statep);
      delete iter;
      return JS_TRUE;
    }
  } FLUSSPFERD_CALLBACK_END;
}

#if JS_VERSION >= 180
void native_object_base::impl::trace_op(
    JSTracer *trc, JSObject *obj)
{
  current_context_scope scope(Impl::wrap_context(trc->context));

  native_object_base &self =
    native_object_base::get_native(Impl::wrap_object(obj));

  tracer tracer_(trc);
  self.trace(tracer_);
}
#else
uint32 native_object_base::impl::mark_op(
    JSContext *ctx, JSObject *obj, void *thing)
{
  current_context_scope scope(Impl::wrap_context(ctx));

  native_object_base &self =
    native_object_base::get_native(Impl::wrap_object(obj));

  tracer trc(thing);
  self.trace(trc);

  return 0;
}
#endif

void native_object_base::call_native_method(
    std::string const &name, call_context &x)
{
  impl::native_method_map::iterator it = p->native_methods.find(name);

  if (it == p->native_methods.end())
    throw exception(("No such method: " + name).c_str());

  impl::method_variant m = it->second;
  switch (m.which()) {
  case 0: // native_method_type
    {
      native_method_type native_method = boost::get<native_method_type>(m);
      if (native_method)
        (this->*native_method)(x);
    }
    break;
  case 1: // callback_type
    {
      callback_type const &callback = boost::get<callback_type>(m);
      if (callback)
        callback(x);
    }
  }
}

void native_object_base::property_op(
    property_mode mode, value const &id, value &data)
{
  std::string name = id.to_string().to_string();
  impl::property_callback_map::iterator it = p->property_callbacks.find(name);

  if (it != p->property_callbacks.end()) {
    property_callback callback = it->second;

    if (callback)
      (this->*callback)(mode, data);
  }
}

bool native_object_base::property_resolve(value const &, unsigned) {
  return false;
}

boost::any native_object_base::enumerate_start(int &n)
{
  n = 0;
  return boost::any();
}

value native_object_base::enumerate_next(boost::any &)
{
  return value();
}

void native_object_base::trace(tracer&) {}
void native_object_base::late_load() {}
