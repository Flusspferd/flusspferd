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

  object::property_attributes attrs;
  bool current = o.has_own_property(p) && o.get_property_attributes(p, attrs);

  bool is_accessor = false, is_data = false,
       configurable = false, enumerable = false, writable = false;

  unsigned flags = object::dont_enumerate;

  // Short circuit if current property is permanent
  if (current) {
    if (attrs.flags & object::permanent_property)
      throw exception("Cannot alter un-configurable properties");

    flags = attrs.flags;
  }


  // [[Enumerable]]. Default false
  value v = desc.get_property("enumerable");
  if (!v.is_void()) {
    if (v.to_boolean() == false) 
      flags |= object::dont_enumerate;
    else {
      flags &= ~object::dont_enumerate;
      enumerable = true;
    }
  }
  
  // [[Configurable]]. Default false
  v = desc.get_property("configurable");
  if (v.is_void() || v.to_boolean() == false)
      flags |= object::permanent_property;
  else {
    flags &= ~object::permanent_property;
    configurable = true;
  }

  // [[Writable]]. Default false
  v = desc.get_property("writable");
  if (!v.is_void() && v.to_boolean() == true) {
    flags &= ~object::read_only_property;
    writable = true;
  }
  else
    flags |= object::read_only_property;

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
  
  is_data = desc.has_property("writable") || desc.has_property("value");
  is_accessor = (getter_fn || setter_fn);
  if ( is_accessor && is_data )
  {
    // 10. If either descObj.[[Getter]] or descObj.[[Setter]] are present, then
    //   a. If either descObj.[[Value]] or descObj.[[Writable]] are present,
    //      then throw a TypeError exception.
    // TODO: This should be a TypeError
    throw exception("cannot mix value or writable with getter or setter");
  }

  // Steps 7, 10a and 11a basically say you can't change anything on a
  // permenant property, but it shouldn't die. That seems like a lot of effort
  // for minimal gain.

  // The rest of the steps basically translate to 'keep the current flags the
  // same', in just a very very verbose manner

  // Wasn't that easy
  if (is_accessor) {
    o.define_property(p, value(), object::property_attributes(flags, getter_fn, setter_fn));
  } else {
    value val = desc.has_property("value") 
              ? desc.get_property("value") 
              : current
              ? o.get_property(p) 
              : value();
    o.define_property(p, val, object::property_attributes(flags));
  }
}

/**
 * Title: Object
 *
 * Extensions to built in Object class.
 *
 * Function: defineProperty
 *
 * Define a property on an object with flags. This allows you to create
 * read-only or non-enumerable properties on your javascript objects.
 *
 * Parameters:
 *   obj - Object on which to define the property
 *   name - property name
 *   descriptor - property descriptor. See below
 *
 * This is modeled after the defineProperty method from the upcoming ES 3.1
 * (draft) specification.
 *
 * A property descriptor is an object with some of the following keys
 *
 *   enumerable - is the property visible in "for in" loops
 * configurable - can further changes be made to this property
 *     writable - read only property
 *        value - initial property value
 *       getter - getter function
 *       setter - setter function
 * 
 * All values default to false or undefined. If either writable or value exist
 * then neither getter nor setter are allowed. 
 *
 * A getter function is one which is called and should return the current value
 * of the property. A setter is called with the argument to set the property
 * too, but you can set it do a derivative of that value, or ignore it
 * completly. Both getter and setter are called with "this" set to the object
 * in question.
 *
 * (code)
 * function MyObj() {
 *   var prop = 0;
 *   Object.defineProperty(
 *     myobj, 
 *     "prop",
 *     { getter: function() { return prop },
 *       setter: function(x) { prop = x * 2; return prop; },
 *       enumerable: false,
 *       configurable: true
 *     }
 *   );
 * }
 * (end code)
 *
 * If configurable is false, then any further attempts to call <defineProperty>
 * on that property will result in an exception.
 */
