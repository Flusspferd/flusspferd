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
  if (!obj.is_null())
    return 0;
  try {
    return flusspferd::get_native<namespace_>(obj).c_obj();
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
  if (!href_v.is_undefined() && !href_v.is_null()) {
    if (href_v.is_string())
      href_p = (xmlChar const *) href_v.get_string().c_str();
    else
      throw exception("Could not create XML namespace: "
                      "href has to be a string");
  }

  xmlChar const *prefix_p = 0;
  if (!prefix_v.is_undefined() && !prefix_v.is_null()) {
    if (prefix_v.is_string())
      prefix_p = (xmlChar const *) prefix_v.get_string().c_str();
    else
      throw exception("Could not create XML namespace: "
                      "prefix has to be a string");
  }

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

  create_native_method(proto, "toString", &namespace_::to_string);

  proto.define_property(
    "href",
    property_attributes(
      permanent_shared_property,
      create_native_method(object(), "", &namespace_::get_href),
      create_native_method(object(), "", &namespace_::set_href)));

  proto.define_property(
    "prefix",
    property_attributes(
      permanent_shared_property,
      create_native_method(object(), "", &namespace_::get_prefix),
      create_native_method(object(), "", &namespace_::set_prefix)));

  return proto;
}

void namespace_::init() {
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

boost::optional<std::string> namespace_::get_href() {
  if (ptr->href)
    return std::string((char const *) ptr->href);
  else
    return boost::none;
}

void namespace_::set_href(boost::optional<std::string> const &x) {
  if (ptr->href)
    xmlFree((xmlChar *) ptr->href);
 
  if (!x)
    ptr->href = 0;
  else
    ptr->href = xmlStrdup((xmlChar const *) x->c_str());
}

boost::optional<std::string> namespace_::get_prefix() {
  if (ptr->href)
    return std::string((char const *) ptr->prefix);
  else
    return boost::none;
}

void namespace_::set_prefix(boost::optional<std::string> const &x) {
  if (ptr->prefix)
    xmlFree((xmlChar *) ptr->prefix);
 
  if (!x)
    ptr->prefix = 0;
  else
    ptr->prefix = xmlStrdup((xmlChar const *) x->c_str());
}
