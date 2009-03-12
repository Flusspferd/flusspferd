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

#include "flusspferd/object.hpp"
#include "flusspferd/property_iterator.hpp"
#include "flusspferd/function.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/root.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/implementation/init.hpp"
#include "flusspferd/implementation/value.hpp"
#include "flusspferd/implementation/object.hpp"
#include <cassert>
#include <js/jsapi.h>

#if JS_VERSION < 180
#include <js/jsobj.h>
#endif

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

void object::set_property(char const *name, value const &v_) {
  if (is_null())
    throw exception("Could not set property (object is null)");
  value v = v_;
  if (!JS_SetProperty(Impl::current_context(), get(), name,
                      Impl::get_jsvalp(v)))
    throw exception("Could not set property");
}

void object::set_property(std::string const &name, value const &v) {
  set_property(name.c_str(), v);
}

void object::set_property(value const &id, value const &v_) {
  if (is_null())
    throw exception("Could not set property (object is null)");
  local_root_scope scope;
  value v = v_;
  string name = id.to_string();
  if (!JS_SetUCProperty(Impl::current_context(), get(),
                        name.data(), name.length(),
                        Impl::get_jsvalp(v)))
    throw exception("Could not set property");
}

value object::get_property(char const *name) const {
  if (is_null())
    throw exception("Could not get property (object is null)");
  value result;
  if (!JS_GetProperty(Impl::current_context(), get_const(),
                      name, Impl::get_jsvalp(result)))
    throw exception("Could not get property");
  return result;
}

value object::get_property(std::string const &name) const {
  return get_property(name.c_str());
}

value object::get_property(value const &id) const {
  if (is_null())
    throw exception("Could not get property (object is null)");
  value result;
  local_root_scope scope;
  string name = id.to_string();
  if (!JS_GetUCProperty(Impl::current_context(), get_const(),
                        name.data(), name.length(),
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

bool object::has_property(value const &id) const {
  if (is_null())
    throw exception("Could not check property (object is null)");
  local_root_scope scope;
  string name = id.to_string();
  JSBool foundp;
  if (!JS_HasUCProperty(Impl::current_context(), get_const(),
                        name.data(), name.length(),
                        &foundp))
    throw exception("Could not check property");
  return foundp;
}

bool object::has_own_property(char const *name_) const {
  local_root_scope scope;
  string name(name_);
  return has_own_property(name);
}

bool object::has_own_property(std::string const &name_) const {
  local_root_scope scope;
  string name(name_);
  return has_own_property(name);
}

bool object::has_own_property(value const &id) const {
  if (is_null())
    throw exception("Could not check property (object is null)");

  JSBool has;
#if JS_VERSION >= 180
  string name = id.to_string();
  if (!JS_AlreadyHasOwnUCProperty(Impl::current_context(), get_const(),
                                  name.data(), name.length(), &has))
#else
  JSObject *obj = get_const();
  jsval argv[] = { Impl::get_jsval(id) };
  jsval vp;
  JSBool ret = js_HasOwnPropertyHelper(Impl::current_context(), obj,
                                       obj->map->ops->lookupProperty, 1, argv, 
                                       &vp);
  has = JSVAL_TO_BOOLEAN(vp);
  if (!ret)
#endif
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

void object::delete_property(value const &id) {
  if (is_null())
    throw exception("Could not delete property (object is null)");
  local_root_scope scope;
  string name = id.to_string();
  jsval dummy;
  if (!JS_DeleteUCProperty2(Impl::current_context(), get(),
                            name.data(), name.length(), &dummy))
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

void object::define_property(
  string const &name, value const &init_value, 
  property_attributes const attrs)
{
  if (is_null())
    throw exception("Could not define property (object is null)");

  unsigned flags = attrs.flags;
  value v;
  v = init_value;

  function getter;
  if (attrs.getter) getter = attrs.getter.get();
  function setter;
  if (attrs.setter) setter = attrs.setter.get();

  JSObject *getter_o = Impl::get_object(getter);
  JSObject *setter_o = Impl::get_object(setter);
   
  unsigned sm_flags = 0;

  if (~flags & dont_enumerate) sm_flags |= JSPROP_ENUMERATE;
  if (flags & read_only_property) sm_flags |= JSPROP_READONLY;
  if (flags & permanent_property) sm_flags |= JSPROP_PERMANENT;
  if (flags & shared_property) sm_flags |= JSPROP_SHARED;

  if (getter_o) sm_flags |= JSPROP_GETTER;
  if (setter_o) sm_flags |= JSPROP_SETTER;

  if(!JS_DefineUCProperty(Impl::current_context(), get_const(),
                          name.data(), name.length(),
                          Impl::get_jsval(v),
                          *(JSPropertyOp*) &getter_o,
                          *(JSPropertyOp*) &setter_o,
                          sm_flags))
    throw exception("Could not define property");
}

void object::define_property(
  std::string const &name_, value const &init_value,
  property_attributes attrs)
{
  local_root_scope scope;
  string name(name_);
  define_property(name, init_value, attrs);
}

void object::define_property(
  char const *name_, value const &init_value,
  property_attributes const attrs)
{
  local_root_scope scope;
  string name(name_);
  define_property(name, init_value,attrs);
}

bool object::is_null() const {
  return !get_const();
}

value object::apply(object const &fn, arguments const &arg_) {
  if (is_null())
    throw exception("Could not apply function (object is null)");

  value fnv(fn);
  root_value result((value()));

  arguments arg(arg_);

  JSBool status = JS_CallFunctionValue(
      Impl::current_context(),
      get(),
      Impl::get_jsval(fnv),
      arg.size(),
      Impl::get_arguments(arg),
      Impl::get_jsvalp(result));

  if (!status)
    throw exception("Could not call function");

  return result;
}

value object::call(char const *fn, arguments const &arg_) {
  if (is_null())
    throw exception("Could not call function (object is null)");

  root_value result((value()));

  arguments arg(arg_);

  JSBool status = JS_CallFunctionName(
      Impl::current_context(),
      get(),
      fn,
      arg.size(),
      Impl::get_arguments(arg),
      Impl::get_jsvalp(result));

  if (!status)
    throw exception("Could not call function");

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

bool object::get_property_attributes(
    char const *name_, property_attributes &attrs)
{
  local_root_scope scope;
  string name(name_);
  return get_property_attributes(name, attrs);
}

bool object::get_property_attributes(
    std::string name_, property_attributes &attrs)
{
  local_root_scope scope;
  string name(name_);
  return get_property_attributes(name, attrs);
}

bool object::get_property_attributes(
    string const &name, property_attributes &attrs)
{
  JSBool found;
  void *getter_op, *setter_op;
  uintN sm_flags;

  attrs.flags = 0;
  attrs.getter = boost::none;
  attrs.setter = boost::none;
  JSBool success = JS_GetUCPropertyAttrsGetterAndSetter(
          Impl::current_context(), get_const(), name.data(), name.length(),
          &sm_flags, &found,
          (JSPropertyOp*)&getter_op, (JSPropertyOp*)&setter_op);

  if (!success)
    throw exception("Could not query property attributes");

  if (!found)
    return false;

  if (~sm_flags & JSPROP_ENUMERATE) attrs.flags |= dont_enumerate;
  if (sm_flags & JSPROP_PERMANENT) attrs.flags |= permanent_property;
  if (sm_flags & JSPROP_READONLY) attrs.flags |= read_only_property;
  if (sm_flags & JSPROP_SHARED) attrs.flags |= shared_property;

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
  return true;
}


object::property_attributes::property_attributes()
  : flags(0u),
    getter(boost::none),
    setter(boost::none)
{}

object::property_attributes::property_attributes(
  unsigned flags, 
  boost::optional<function const &> getter,
  boost::optional<function const &> setter
)
  : flags(flags),
    getter(getter),
    setter(setter)
{}
