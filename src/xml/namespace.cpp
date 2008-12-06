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

#include "flusspferd/xml/namespace.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/local_root_scope.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

xmlNsPtr namespace_::c_from_js(object const &obj) {
  if (!obj.is_valid())
    return 0;
  try {
    namespace_ &p =
      dynamic_cast<namespace_&>(native_object_base::get_native(obj));
    return p.c_obj();
  } catch (std::exception&) {
    return 0;
  }
}

object namespace_::create(xmlNsPtr ptr) {
  if (!ptr)
    return object();
  if (ptr->_private)
    return *static_cast<object *>(ptr->_private);
  return create_native_object<namespace_>(object(), ptr);
}

namespace_::namespace_(object const &obj, xmlNsPtr ptr)
  : native_object_base(obj), ptr(ptr)
{
  if (!ptr->_private)
    ptr->_private = static_cast<object *>(this);
  init();
}

namespace_::namespace_(object const &obj, call_context &x)
  : native_object_base(obj)
{
  local_root_scope scope;

  value node = x.arg[0];
  value href_v = x.arg[1];
  value prefix_v = x.arg[2];

  if (!node.is_object())
    throw exception("Could not create XML namespace: "
                    "node has to be an object");

  xmlNodePtr node_p = node::c_from_js(node.get_object());

  if (!node_p && !node.is_null())
    throw exception("Could not create XML namespace: "
                    "node has to be null or an XML node");

  xmlChar const *href_p = 0;
  if (!href_v.is_void() && !href_v.is_null())
    if (href_v.is_string())
      href_p = (xmlChar const *) href_v.get_string().c_str();
    else
      throw exception("Could not create XML namespace: "
                      "href has to be a string");

  xmlChar const *prefix_p = 0;
  if (!prefix_v.is_void() && !prefix_v.is_null())
    if (prefix_v.is_string())
      prefix_p = (xmlChar const *) prefix_v.get_string().c_str();
    else
      throw exception("Could not create XML namespace: "
                      "prefix has to be a string");

  ptr = xmlNewNs(node_p, href_p, prefix_p);

  if (!ptr)
    throw exception("Could not create XML namespace");

  ptr->_private = static_cast<object *>(this);

  init();
}

namespace_::~namespace_() {
  if (ptr && !ptr->context && ptr->_private == static_cast<object *>(this)) {
    xmlFreeNs(ptr);
  }
}

object namespace_::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "toString", 1);

  return proto;
}

void namespace_::init() {
  register_native_method("toString", &namespace_::to_string);

  unsigned const RW = permanent_shared_property;

  define_native_property("href", RW, &namespace_::prop_href);
  define_native_property("prefix", RW, &namespace_::prop_prefix);
}

void namespace_::trace(tracer &trc) {
  trc("namespace-self", *static_cast<object*>(ptr->_private));
  if (ptr->context)
    trc("namespace-context", *static_cast<object*>(ptr->context->_private));
}

string namespace_::to_string() {
  std::string text;

  if (ptr->href)
    text += (char const *) ptr->href;
  else
    text += "<>";

  if (ptr->prefix) {
    text += " (";
    text += (char const *) ptr->prefix;
    text += ")";
  }

  return string(text);
}

void namespace_::prop_href(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    if (ptr->href)
      data = string((char const *) ptr->href);
    else
      data = value();
    break;
  case property_set:
    if (ptr->href)
      xmlFree((xmlChar *) ptr->href);
    if (data.is_void() || data.is_null())
      ptr->href = 0;
    else
      ptr->href = xmlStrdup((xmlChar const *) data.to_string().c_str());
    break;
  default: break;
  }
}

void namespace_::prop_prefix(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    if (ptr->prefix)
      data = string((char const *) ptr->prefix);
    else
      data = value();
    break;
  case property_set:
    if (ptr->prefix)
      xmlFree((xmlChar *) ptr->prefix);
    if (data.is_void() || data.is_null())
      ptr->prefix = 0;
    else
      ptr->prefix = xmlStrdup((xmlChar const *) data.to_string().c_str());
    break;
  default: break;
  }
}
