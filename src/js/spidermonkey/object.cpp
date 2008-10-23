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

#include "templar/js/object.hpp"
#include "templar/js/function.hpp"
#include "templar/js/exception.hpp"
#include "templar/js/local_root_scope.hpp"
#include "templar/js/root_value.hpp"
#include "templar/js/arguments.hpp"
#include "templar/js/string.hpp"
#include "templar/js/native_object_base.hpp"
#include "templar/js/spidermonkey/init.hpp"
#include "templar/js/spidermonkey/value.hpp"
#include "templar/js/spidermonkey/object.hpp"
#include <cassert>
#include <js/jsapi.h>

namespace templar { namespace js {
  object::object() : Impl::object_impl(0) { }
  object::~object() { }

  object object::get_parent() {
    return Impl::wrap_object(
      JS_GetParent(Impl::current_context(), get()));
  }

  object object::get_prototype() {
    return Impl::wrap_object(
      JS_GetPrototype(Impl::current_context(), get()));
  }

  void object::set_property(char const *name, value const &v) {
    if(!JS_SetProperty(Impl::current_context(), get(), name,
                       Impl::get_jsvalp(const_cast<value&>(v))))
      throw exception("Could not set property");
  }
  void object::set_property(std::string const &name, value const &v) {
    set_property(name.c_str(), v);
  }

  value object::get_property(char const *name) const {
    value result;
    object &self = *const_cast<object*>(this);
    if (!JS_GetProperty(Impl::current_context(), self.get(),
                        name, Impl::get_jsvalp(result)))
      throw exception("Could not get property");
    return result;
  }
  value object::get_property(std::string const &name) const {
    return get_property(name.c_str());
  }

  bool object::has_property(char const *name) const {
    JSBool foundp;
    if(!JS_HasProperty(Impl::current_context(), get_const(), name,
                       &foundp))
      throw exception("Could not check property");
    return foundp;
  }
  bool object::has_property(std::string const &name) const {
    return has_property(name.c_str());
  }

  void object::delete_property(char const *name) {
    if(!JS_DeleteProperty(Impl::current_context(), get(), name))
      throw exception("couldn't delete property");
  }
  void object::delete_property(std::string const &name) {
    delete_property(name.c_str());
  }

  namespace Impl {
    object_impl::property_iterator_impl &
    object_impl::property_iterator_impl::operator++() {
      if(!JS_NextProperty(Impl::current_context(), iter, &id))
        throw exception("couldn't get next property");
      return *this;
    }
  }

  object::property_iterator &object::property_iterator::operator++() {
    ++*static_cast<Impl::object_impl::property_iterator_impl*>(this);
    return *this;
  }

  bool operator==(object::property_iterator const &lhs,
                  object::property_iterator const &rhs)
  {
    if(!lhs.get_const() && rhs.get_const())
      return rhs.get_id() == JSVAL_VOID;
    else if(!rhs.get_const() && lhs.get_const())
      return rhs.get_id() == JSVAL_VOID;
    else
      return rhs.get_const() == lhs.get_const();
  }

  object::property_iterator object::begin() const {
    JSObject *obj = JS_NewPropertyIterator(Impl::current_context(),
                                           get_const());
    if(!obj)
      throw exception("couldn't create property_iterator");
    return Impl::object_impl::property_iterator_impl(obj);
  }
  object::property_iterator object::end() const {
    return property_iterator(
      Impl::object_impl::property_iterator_impl());
  }

  std::string object::property_iterator::operator*() const {
    assert(get_id() != JSVAL_VOID);
    value val;
    if(!JS_IdToValue(Impl::current_context(), get_id(), 
                     Impl::get_jsvalp(val)))
      throw exception("couldn't convert id to value");
    if(!val.is_string()) // TODO: could be an "index"!
      throw exception("unexpected type");
    return val.to_string().to_string();
  }

  void object::define_property(char const *name,
                               value const & init_value,
                               unsigned flags,
                               boost::optional<function const &> getter_,
                               boost::optional<function const &> setter_)
  {
    value v;
    v = init_value;

    function getter;
    if (getter_) getter = getter_.get();
    function setter;
    if (setter_) setter = setter_.get();

    JSObject *getter_o = Impl::get_object(getter);
    JSObject *setter_o = Impl::get_object(setter);
    
    unsigned sm_flags = 0;

    if (~flags & dont_enumerate) sm_flags |= JSPROP_ENUMERATE;
    if (flags & read_only_property) sm_flags |= JSPROP_READONLY;
    if (flags & permanent_property) sm_flags |= JSPROP_PERMANENT;

    if (getter_o) sm_flags |= JSPROP_GETTER;
    if (setter_o) sm_flags |= JSPROP_SETTER;

    if(!JS_DefineProperty(Impl::current_context(), get_const(),
                          name, Impl::get_jsval(v),
                          (JSPropertyOp) getter_o,
                          (JSPropertyOp) setter_o,
                          sm_flags))
      throw exception("Could not define property");
  }

  void object::define_property(std::string const &name,
                               value const &init_value,
                               unsigned flags,
                               boost::optional<function const &> getter,
                               boost::optional<function const &> setter)
  {
    define_property(name.c_str(), init_value, flags, getter, setter);
  }

  bool object::is_valid() const {
    return get_const();
  }

  value object::apply(object const &fn, arguments const &arg_) {
    value fnv(fn);
    root_value result((value()));

    arguments arg(arg_);

    JSBool status = JS_CallFunctionValue(
        Impl::current_context(),
        get(),
        Impl::get_jsval(fnv),
        arg.size(),
        arg.get(),
        Impl::get_jsvalp(result));

    if (!status)
      throw exception("Could not call function");

    return result;
  }

  value object::call(char const *fn, arguments const &arg_) {
    root_value result((value()));

    arguments arg(arg_);

    JSBool status = JS_CallFunctionName(
        Impl::current_context(),
        get(),
        fn,
        arg.size(),
        arg.get(),
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

  object object::create() {
    local_root_scope scope;

    JSObject *o = JS_NewObject(Impl::current_context(), 0, 0, 0);
    if (!o)
      throw exception("Could not create object");

    return Impl::wrap_object(o);
  }

  object object::create_array(unsigned length) {
    local_root_scope scope;

    JSObject *o = JS_NewArrayObject(Impl::current_context(), length, 0);
    if (!o)
       throw exception("Could not create array");

    return Impl::wrap_object(o);
  }

  object object::create_native(native_object_base *ptr) {
    try {
      return ptr->create_object();
    } catch (...) {
      delete ptr;
      throw;
    }
  }
}}
