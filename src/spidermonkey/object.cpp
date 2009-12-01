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

#include "flusspferd/object.hpp"
#include "flusspferd/property_iterator.hpp"
#include "flusspferd/function.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/root.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/value.hpp"
#include "flusspferd/spidermonkey/object.hpp"
#include <js/jsapi.h>

using namespace flusspferd;

object::object() : Impl::object_impl(0) { }
object::~object() { }

void object::seal(bool deep) {
  if (is_null())
    throw exception("Could not seal object (object is null)");
  if (!JS_SealObject(Impl::current_context(), get(), deep))
    throw exception("Could not seal object");
}

object object::parent() {
  if (is_null())
    throw exception("Could not get object parent (object is null)");
  return Impl::wrap_object(JS_GetParent(Impl::current_context(), get()));
}

object object::prototype() {
  if (is_null())
    throw exception("Could not get object prototype (object is null)");
  return Impl::wrap_object(JS_GetPrototype(Impl::current_context(), get()));
}

void object::set_parent(object const &o) {
  if (is_null())
    throw exception("Could not set parent (object is null)");
  if (!JS_SetParent(Impl::current_context(), get(), o.get_const()))
    throw exception("Could not set object parent");
}

void object::set_prototype(object const &o) {
  if (is_null())
    throw exception("Could not set object prototype (object is null)");
  if (!JS_SetPrototype(Impl::current_context(), get(), o.get_const()))
    throw exception("Could not set object prototype");
}

value object::set_property(char const *name, value const &v_) {
  if (is_null())
    throw exception("Could not set property (object is null)");
  root_value v(v_);
  if (!JS_SetProperty(Impl::current_context(), get(), name,
                      Impl::get_jsvalp(v)))
    throw exception("Could not set property");
  return v;
}

value object::set_property(std::string const &name, value const &v) {
  return set_property(name.c_str(), v);
}

value object::set_property(value const &id_, value const &v_) {
  if (is_null())
    throw exception("Could not set property (object is null)");
  root_value id(id_);
  root_value v(v_);
  if (!JS_SetPropertyById(Impl::current_context(), get(),
                          Impl::get_jsid(id),
                          Impl::get_jsvalp(v)))
    throw exception("Could not set property");
  return v;
}

value object::get_property(char const *name) const {
  if (is_null())
    throw exception("Could not get property (object is null)");
  root_value result;
  if (!JS_GetProperty(Impl::current_context(), get_const(),
                      name, Impl::get_jsvalp(result)))
    throw exception("Could not get property");
  return result;
}

value object::get_property(std::string const &name) const {
  return get_property(name.c_str());
}

value object::get_property(value const &id_) const {
  if (is_null())
    throw exception("Could not get property (object is null)");
  root_value result;
  root_value id(id_);
  if (!JS_GetPropertyById(Impl::current_context(), get_const(),
                          Impl::get_jsid(id),
                          Impl::get_jsvalp(result)))
    throw exception("Could not get property");
  return result;
}

bool object::has_property(char const *name) const {
  if (is_null())
    throw exception("Could not check property (object is null)");
  JSBool foundp;
  if (!JS_HasProperty(Impl::current_context(), get_const(), name,
                      &foundp))
    throw exception("Could not check property");
  return foundp;
}

bool object::has_property(std::string const &name) const {
  return has_property(name.c_str());
}

bool object::has_property(value const &id_) const {
  if (is_null())
    throw exception("Could not check property (object is null)");
  root_value id(id_);
  JSBool foundp;
  if (!JS_HasPropertyById(Impl::current_context(), get_const(),
                          Impl::get_jsid(id),
                          &foundp))
    throw exception("Could not check property");
  return foundp;
}

bool object::has_own_property(char const *name_) const {
  root_string name(name_);
  return has_own_property(name);
}

bool object::has_own_property(std::string const &name_) const {
  root_string name(name_);
  return has_own_property(name);
}

bool object::has_own_property(value const &id) const {
  if (is_null())
    throw exception("Could not check property (object is null)");

  JSBool has;
  string name = id.to_string();
  if (!JS_AlreadyHasOwnPropertyById(Impl::current_context(), get_const(),
                                    Impl::get_jsid(id), &has))
  {
    throw exception("Unable to check for own property");
  }
  return has;
}

void object::delete_property(char const *name) {
  if (is_null())
    throw exception("Could not delete property (object is null)");
  if (!JS_DeleteProperty(Impl::current_context(), get(), name))
    throw exception("Could not delete property");
}

void object::delete_property(std::string const &name) {
  delete_property(name.c_str());
}

void object::delete_property(value const &id_) {
  if (is_null())
    throw exception("Could not delete property (object is null)");
  root_value id(id_);
  jsval dummy;
  if (!JS_DeletePropertyById2(Impl::current_context(), get(),
                              Impl::get_jsid(id), &dummy))
    throw exception("Could not delete property");
}

property_iterator object::begin() const {
  if (is_null())
    throw exception("Could not create iterator for null object");
  return property_iterator(*this);
}

property_iterator object::end() const {
  if (is_null())
    throw exception("Could not create iterator for null object");
  return property_iterator();
}

namespace {
  /// common function of define_property
  void extract_attributes(
    property_attributes const &attrs,
    function &getter,
    function &setter,
    unsigned &sm_flags)
  {
    if (attrs.getter) getter = attrs.getter.get();
    if (attrs.setter) setter = attrs.setter.get();

    unsigned const flags = attrs.flags;
    if (~flags & dont_enumerate) sm_flags |= JSPROP_ENUMERATE;
    if (flags & read_only_property) sm_flags |= JSPROP_READONLY;
    if (flags & permanent_property) sm_flags |= JSPROP_PERMANENT;
    if (flags & shared_property) sm_flags |= JSPROP_SHARED;

    if (!getter.is_null()) sm_flags |= JSPROP_GETTER;
    if (!setter.is_null()) sm_flags |= JSPROP_SETTER;
  }
}

void object::define_property(
  value const &id,
  value const &init_value, 
  property_attributes const &attrs)
{
  if (is_null())
    throw exception("Could not define property (object is null)");

  root_value id_r(id);
  root_value v(init_value);

  root_function getter;
  root_function setter;
  unsigned sm_flags = 0;
  extract_attributes(attrs, getter, setter, sm_flags);

  JSObject *getter_o = Impl::get_object(getter);
  JSObject *setter_o = Impl::get_object(setter);

  if (!JS_DefinePropertyById(Impl::current_context(),
                             get_const(),
                             Impl::get_jsid(id),
                             Impl::get_jsval(v),
                             *(JSPropertyOp*) &getter_o,
                             *(JSPropertyOp*) &setter_o,
                             sm_flags))
    throw exception("Could not define property");
}

void object::define_property(
  string const &name,
  value const &init_value, 
  property_attributes const &attrs)
{
  if (is_null())
    throw exception("Could not define property (object is null)");

  root_string name_r(name);
  root_value v(init_value);

  root_function getter;
  root_function setter;
  unsigned sm_flags = 0;
  extract_attributes(attrs, getter, setter, sm_flags);

  JSObject *getter_o = Impl::get_object(getter);
  JSObject *setter_o = Impl::get_object(setter);

  if (!JS_DefineUCProperty(Impl::current_context(),
                           get_const(),
                           name.data(), name.length(),
                           Impl::get_jsval(v),
                           *(JSPropertyOp*) &getter_o,
                           *(JSPropertyOp*) &setter_o,
                           sm_flags))
    throw exception("Could not define property");
}

bool object::is_null() const {
  return !get_const();
}

value object::apply(object const &fn, arguments const &arg_) {
  if (is_null())
    throw exception("Could not apply function (object is null)");

  if (fn.is_null())
    throw exception("Could not apply function (function is null)");

  value fnv(fn);
  root_value result((value()));

  arguments arg(arg_);

  JSContext *cx = Impl::current_context();

  JSBool status = JS_CallFunctionValue(
      cx,
      get(),
      Impl::get_jsval(fnv),
      arg.size(),
      Impl::get_arguments(arg),
      Impl::get_jsvalp(result));

  if (!status) {
    if (JS_IsExceptionPending(cx))
      throw exception("Could not call function");
    else
      throw js_quit();
  }

  return result;
}

value object::call(char const *fn, arguments const &arg_) {
  if (is_null())
    throw exception("Could not call function (object is null)");

  root_value result((value()));

  arguments arg(arg_);

  JSContext *cx = Impl::current_context();

  JSBool status = JS_CallFunctionName(
      cx,
      get(),
      fn,
      arg.size(),
      Impl::get_arguments(arg),
      Impl::get_jsvalp(result));

  if (!status) {
    if (JS_IsExceptionPending(cx))
      throw exception("Could not call function");
    else
      throw js_quit();
  }

  return result;
}

value object::call(std::string const &fn, arguments const &arg) {
  return call(fn.c_str(), arg);
}

value object::call(object o, arguments const &arg) {
  return o.apply(*this, arg);
}

value object::call(arguments const &arg) {
  return call(global(), arg);
}

bool object::is_array() const {
  if (is_null())
    return false;
  return JS_IsArrayObject(Impl::current_context(), get_const());
}

bool object::is_generator() const {
  if (is_null())
    return false;

  JSContext *ctx = Impl::current_context();
  (void)ctx;

  // There seems to be no way with the SM API to get the standard Generator
  // JSClass back. SO make a best effort guess
  JSClass *our_class = JS_GET_CLASS(ctx, get_const());

  return strcmp(our_class->name, "Generator") == 0
      && get_property("next").is_function();
}

bool object::instance_of(value constructor) const {
  root_object cons(constructor.to_object());

  if (cons.is_null())
    throw exception("Could not check instance constructor: "
                    "constructor has to be a non-null object");

  JSBool result;
  if (!JS_HasInstance(
          Impl::current_context(),
          Impl::get_object(cons),
          Impl::get_jsval(value(*this)),
          &result))
    throw exception("Could not check instance constructor");

  return result;
}

namespace {
  property_attributes
  set_property_attributes(
    uintN sm_flags, void *getter_op, void *setter_op)
  {
    property_attributes attrs;
    if (~sm_flags & JSPROP_ENUMERATE)
      attrs.flags = attrs.flags | dont_enumerate;
    if (sm_flags & JSPROP_PERMANENT)
      attrs.flags = attrs.flags | permanent_property;
    if (sm_flags & JSPROP_READONLY)
      attrs.flags = attrs.flags | read_only_property;
    if (sm_flags & JSPROP_SHARED)
      attrs.flags = attrs.flags | shared_property;

    if (getter_op) {
      if (sm_flags & JSPROP_GETTER) {
        attrs.getter = Impl::wrap_object((JSObject*)getter_op);
      } else {
        // What do i set attrs.getter to here....?
      }
    }
  
    if (setter_op) {
      if (sm_flags & JSPROP_SETTER) {
        attrs.setter = Impl::wrap_object((JSObject*)setter_op);
      } else {
        // What do i set attrs.setter to here....?
      }
    }
    return attrs;
  }
}

boost::optional<property_attributes>
object::get_property_attributes(string const &name) const {
  if (is_null())
    throw exception("Could not get property attributes (object is null)");

  flusspferd::root_string name_r(name);

  JSBool found;
  void *getter_op, *setter_op;
  uintN sm_flags;
  JSBool success = JS_GetUCPropertyAttrsGetterAndSetter(
          Impl::current_context(), get_const(), name.data(), name.length(),
          &sm_flags, &found,
          (JSPropertyOp*)&getter_op, (JSPropertyOp*)&setter_op);

  if (!success)
    throw exception("Could not query property attributes");

  if (!found)
    return boost::none;

  return set_property_attributes(sm_flags, getter_op, setter_op);
}

boost::optional<property_attributes>
object::get_property_attributes(value const &id) const {
  if (is_null())
    throw exception("Could not get property attributes (object is null)");

  flusspferd::root_value id_r(id);

  JSBool found;
  void *getter_op, *setter_op;
  uintN sm_flags;
  JSBool success = JS_GetPropertyAttrsGetterAndSetterById(
          Impl::current_context(), get_const(), Impl::get_jsid(id),
          &sm_flags, &found,
          (JSPropertyOp*)&getter_op, (JSPropertyOp*)&setter_op);

  if (!success)
    throw exception("Could not query property attributes");

  if (!found)
    return boost::none;

  return set_property_attributes(sm_flags, getter_op, setter_op);
}
