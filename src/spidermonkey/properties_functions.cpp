// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld, Ash Berlin

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
#include "flusspferd/properties_functions.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"

using namespace flusspferd;

// An implementation like [[DefineOwnProperty]] from ES3.1 spec
void ecma_define_own_property(object self, object o, string p, object desc);

void flusspferd::load_properties_functions(object container) {
  value v = container.get_property("Object");

  if (!v.is_object())
    throw exception("Unable to get Object constructor");
  object obj_ctor = v.to_object();

  create_native_function(obj_ctor, "defineProperty", ecma_define_own_property);
}


void ecma_define_own_property(object /*self*/, object o, string p, object desc) {
  local_root_scope scope;

  // TODO: Check if obj is sealed
 
  bool current = o.has_property(p);

  // TODO: Short circuit if current property is permanent

  unsigned flags = object::read_only_property;
  bool writable = false,
       is_accessor = false;

  // [[Enumerable]]. Default false
  value v = desc.get_property("enumerable");
  if (!v.is_void() && v.get_boolean() == false) 
      flags |= object::dont_enumerate;
  
  // [[Configurable]]. Default false
  v = desc.get_property("configurable");
  if (!v.is_void() && v.get_boolean() == true)
    flags |= object::permanent_property;

  // [[Writable]]. Default false
  v = desc.get_property("writable");
  if (!v.is_void() && v.get_boolean() == true) {
    writable = true;
    flags &= ~object::read_only_property;
  }

  boost::optional<function const &> getter_fn = boost::none, 
                                    setter_fn = boost::none;
  v = desc.get_property("getter");
  try {
    if (!v.is_void())
      getter_fn = v.to_object();
  } catch (exception &e) {
    // TODO: This should be a TypeError
    throw exception("getter must be a function");
  }

  v = desc.get_property("setter");
  try {
    if (!v.is_void())
      setter_fn = v.to_object();
  } catch (exception &e) {
    // TODO: This should be a TypeError
    throw exception("setter must be a function");
  }
  
  is_accessor = (getter_fn || setter_fn);
  if ( is_accessor && 
       (desc.has_property("value") || desc.has_property("writable")) )
  {
    // 10. If either descObj.[[Getter]] or descObj.[[Setter]] are present, then
    //   a. If either descObj.[[Value]] or descObj.[[Writable]] are present,
    //      then throw a TypeError exception.
    // TODO: This should be a TypeError
    throw exception("cannot mix value or writable with getter or setter");
  }

  if (!current) {
    if (is_accessor) {
      o.define_property(p, value(), flags, getter_fn, setter_fn);
    } else {
      value val = desc.get_property("value");
      o.define_property(p, val, flags);
    }
    return;
  }
}
