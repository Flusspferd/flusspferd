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
#include "flusspferd/properties_functions.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"

using namespace flusspferd;

// An implementation like [[DefineOwnProperty]] from ES5 spec 8.12.10
void ecma_define_own_property(object o, string p, object desc);

void flusspferd::load_properties_functions(object container) {
  value v = container.get_property("Object");

  if (!v.is_object())
    throw exception("Unable to get Object constructor");
  object obj_ctor = v.to_object();

  create_native_function(
      obj_ctor,
      "defineProperty",
      ecma_define_own_property);
}


// This doesn't quite match the exact behaviour of the spec, but it hopefully
// captures the intent.
void ecma_define_own_property(object o, string p, object desc) {
  local_root_scope scope;

  // TODO: Check if obj is sealed/not extensible

  property_attributes attrs;
  bool current = o.has_own_property(p) && o.get_property_attributes(p, attrs);

  bool is_accessor = false, is_data = false,
       configurable = false, enumerable = false, writable = false;

  property_flag flags = dont_enumerate;

  // Short circuit if current property is permanent
  if (current) {
    if (attrs.flags & permanent_property)
      throw exception("Cannot alter un-configurable properties");

    flags = attrs.flags;
  }


  // [[Enumerable]]. Default false
  value v = desc.get_property("enumerable");
  if (!v.is_undefined()) {
    if (v.to_boolean() == false) 
      flags = flags | dont_enumerate;
    else {
      flags = flags & ~dont_enumerate;
      enumerable = true;
    }
  }
  
  // [[Configurable]]. Default false
  v = desc.get_property("configurable");
  if (v.is_undefined() || v.to_boolean() == false)
      flags = flags | permanent_property;
  else {
    flags = flags & ~permanent_property;
    configurable = true;
  }

  // [[Writable]]. Default false
  v = desc.get_property("writable");
  if (!v.is_undefined() && v.to_boolean() == true) {
    flags = flags & ~read_only_property;
    writable = true;
  }
  else
    flags = flags | read_only_property;

  boost::optional<function> getter_fn = boost::none, 
                            setter_fn = boost::none;
  v = desc.get_property("getter");
  try {
    if (!v.is_undefined())
      getter_fn = v.to_object();
  } catch (exception &) {
    throw exception("getter must be a function", "TypeError");
  }

  v = desc.get_property("setter");
  try {
    if (!v.is_undefined()) {
      setter_fn = v.to_object();
      flags = flags &~ read_only_property;
    }
  } catch (exception &) {
    throw exception("setter must be a function", "TypeError");
  }

  is_data = desc.has_property("writable") || desc.has_property("value");
  is_accessor = (getter_fn || setter_fn);
  if ( is_accessor && is_data )
  { // S.8.10.5
    // 9. If either desc.[[Get]] or desc.[[Set]] are present, then
    //   a. If either desc.[[Value]] or desc.[[Writable]] are present,
    //      then throw a TypeError exception.
    throw exception(
      "cannot mix value or writable with getter or setter",
      "TypeError");
  }

  // Steps 7, 10a and 11a basically say you can't change anything on a
  // permenant property, but it shouldn't die. That seems like a lot of effort
  // for minimal gain.

  // The rest of the steps basically translate to 'keep the current flags the
  // same', in just a very very verbose manner

  // Wasn't that easy
  if (is_accessor) {
    o.define_property(
      p,
      property_attributes(
        flags,
        boost::optional<function const &>(getter_fn),
        boost::optional<function const &>(setter_fn)));
  } else {
    value val = desc.has_property("value")
              ? desc.get_property("value") 
              : current
              ? o.get_property(p)
              : value();
    o.define_property(p, val, property_attributes(flags));
  }
}
