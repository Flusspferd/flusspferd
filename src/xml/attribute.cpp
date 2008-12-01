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

#include "flusspferd/xml/attribute.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/xml/namespace.hpp"
#include "flusspferd/xml/text.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

attribute_::attribute_(xmlAttrPtr ptr)
  : node(xmlNodePtr(ptr))
{}

static xmlAttrPtr new_attribute_(call_context &x) {
  local_root_scope scope;

  int offset = 0;

  xmlNodePtr parent = node::c_from_js(x.arg[offset + 0].to_object());
  offset -= !parent;

  value name_v = x.arg[offset + 1];

  if (!name_v.is_string())
    throw exception("Could not create XML element attribute: "
                    "name has to be a string");

  xmlChar const *name = (xmlChar const *) name_v.get_string().c_str();

  xmlNsPtr ns = namespace_::c_from_js(x.arg[offset + 2].to_object());

  if (!ns && !x.arg[offset + 2].is_void_or_null())
    --offset;

  value content_v = x.arg[offset + 3];

  if (!content_v.is_void_or_null() && !content_v.is_string())
    throw exception("Could not create XML element attribute: "
                    "content has to be a string");

  xmlChar const *content = 0;
  
  if (!content_v.is_void_or_null())
    content = (xmlChar const *) content_v.get_string().c_str();

  xmlAttrPtr result = xmlNewNsProp(parent, ns, name, content);

  if (!result)
    throw exception("Could not create XML element attribute");

  return result;
}

attribute_::attribute_(call_context &x)
  : node(xmlNodePtr(new_attribute_(x)))
{}

attribute_::~attribute_()
{}

void attribute_::post_initialize() {
  node::post_initialize();

  register_native_method("addContent", &attribute_::add_content);

  define_native_property("content",
      permanent_shared_property,
      &attribute_::prop_content);
}

object attribute_::class_info::create_prototype() {
  local_root_scope scope;

  object proto = node::class_info::create_prototype();

  create_native_method(proto, "addContent", 1);

  return proto;
}

void attribute_::add_content(string const &content) {
  if (content.empty())
    return;

  local_root_scope scope;
  call_context x;
  x.arg.push_back(content);

  object txt = create_native_object<text>(object(), boost::ref(x));

  arguments arg;
  arg.push_back(txt);

  call("addChild", arg);
}

void attribute_::prop_content(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    xmlNodeSetContent(node::c_obj(), 0);
    if (!data.is_void_or_null())
      add_content(data.to_string());
    // !! fall thru !!
  case property_get: {
    xmlChar *content = xmlNodeGetContent(node::c_obj());
    if (!content) {
      data = value();
    } else {
      data = string((char const *) content);
      xmlFree(content);
    }
    break; }
  default: break;
  }
}
