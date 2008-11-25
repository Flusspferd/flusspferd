// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated nodeation files (the "Software"), to deal
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

#include "flusspferd/xml/node.hpp"
#include "flusspferd/xml/document.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/local_root_scope.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

object node::create(xmlNodePtr ptr) {
  switch (ptr->type) {
  case XML_DOCUMENT_NODE:
    return create_native_object<document>(object(), xmlDocPtr(ptr));
  default:
    return create_native_object<node>(object(), ptr);
  }
}

node::node(xmlNodePtr ptr)
  : ptr(ptr)
{
  if (!ptr->_private)
    ptr->_private = get_gcptr();
}

node::node(call_context &x) {
  local_root_scope scope;

  string name(x.arg[0]);

  ptr = xmlNewNode(0, (xmlChar const *) name.c_str());

  if (!ptr)
    throw exception("Could not create XML node");

  ptr->_private = get_gcptr();
}

node::~node() {
  if (ptr && !ptr->parent && ptr->_private == get_gcptr()) {
    xmlFreeNode(ptr);
  }
}

void node::post_initialize() {
  register_native_method("copy", &node::copy);

  define_native_property("name", permanent_property, &node::prop_name);
  define_native_property("lang", permanent_property, &node::prop_lang);
  define_native_property(
    "document", permanent_property | read_only_property, &node::prop_document);
}

object node::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "copy", 1);

  return proto;
}

char const *node::class_info::constructor_name() {
  return "Node";
}

std::size_t node::class_info::constructor_arity() {
  return 1;
}

static void trace_children(tracer &trc, xmlNodePtr ptr) {
  while (ptr) {
    trc("node-children", ptr->_private);
    trace_children(trc, ptr->children);
    ptr = ptr->next;
  }
}

static void trace_parents(tracer &trc, xmlNodePtr ptr) {
  while (ptr) {
    trc("node-parent", ptr->_private);
    ptr = ptr->parent;
  }
}

void node::trace(tracer &trc) {
  trc("node-self", ptr->_private);

  trace_children(trc, ptr->children);
  trace_parents(trc, ptr->parent);

  if (ptr->doc)
    trc("node-doc", ptr->doc->_private);

  if (ptr->next)
    trc("node-next", ptr->next->_private);

  if (ptr->prev)
    trc("node-prev", ptr->prev->_private);
}

void node::prop_name(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    if (!ptr->name)
      data = value();
    else
      data = string((char const *) ptr->name);
    break;
  case property_set:
    xmlNodeSetName(ptr, (xmlChar const *) data.to_string().c_str());
    break;
  default: break;
  };
}

void node::prop_lang(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    {
      xmlChar const *lang = xmlNodeGetLang(ptr);
      if (!lang)
        data = value();
      else
        data = string((char const *) lang);
    }
    break;
  case property_set:
    xmlNodeSetLang(ptr, (xmlChar const *) data.to_string().c_str());
    break;
  default: break;
  };
}

void node::prop_document(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  if (!ptr->doc)
    data = object();
  else
    data = create_native_object<document>(object(), ptr->doc);
}

object node::copy(bool recursive) {
  xmlNodePtr copy = xmlCopyNode(ptr, recursive);

  if (!copy)
    throw exception("Could not copy XML node");

  return create(copy);
}

