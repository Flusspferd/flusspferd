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
  if (ptr->_private)
    return from_permanent_ptr(ptr->_private);

  switch (ptr->type) {
  case XML_DOCUMENT_NODE:
    return create_native_object<document>(object(), xmlDocPtr(ptr));
  default:
    return create_native_object<node>(object(), ptr);
  }
}

xmlNodePtr node::c_from_js(object const &obj) {
  if (!obj.is_valid())
    return 0;
  try {
    xml::node *p =
      dynamic_cast<xml::node*>(native_object_base::get_native(obj));
    if (!p)
      return 0;
    return p->c_obj();
  } catch (std::exception&) {
    return 0;
  }
}

node::node(xmlNodePtr ptr)
  : ptr(ptr)
{
  ptr->_private = permanent_ptr();
}

node::node(call_context &x) {
  local_root_scope scope;

  string name(x.arg[0]);

  ptr = xmlNewNode(0, (xmlChar const *) name.c_str());

  if (!ptr)
    throw exception("Could not create XML node");

  ptr->_private = permanent_ptr();
}

node::~node() {
  if (ptr && !ptr->parent && ptr->_private == permanent_ptr()) {
    xmlFreeNode(ptr);
  }
}

void node::post_initialize() {
  ptr->_private = permanent_ptr();

  register_native_method("copy", &node::copy);

  define_native_property("name", permanent_property, &node::prop_name);
  define_native_property("lang", permanent_property, &node::prop_lang);
  define_native_property("parent", permanent_property, &node::prop_parent);
  define_native_property("next", permanent_property, &node::prop_next);
  define_native_property("prev", permanent_property, &node::prop_prev);
  define_native_property(
    "document", permanent_property | read_only_property, &node::prop_document);
  define_native_property(
    "type", permanent_property | read_only_property, &node::prop_type);
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
    trc("node-children", value::from_permanent_ptr(ptr->_private));
    trace_children(trc, ptr->children);
    ptr = ptr->next;
  }
}

static void trace_parents(tracer &trc, xmlNodePtr ptr) {
  while (ptr) {
    trc("node-parent", value::from_permanent_ptr(ptr->_private));
    ptr = ptr->parent;
  }
}

void node::trace(tracer &trc) {
  trc("node-self", value::from_permanent_ptr(ptr->_private));

  trace_children(trc, ptr->children);
  trace_parents(trc, ptr->parent);

  if (ptr->doc)
    trc("node-doc", value::from_permanent_ptr(ptr->doc->_private));

  if (ptr->next)
    trc("node-next", value::from_permanent_ptr(ptr->next->_private));

  if (ptr->prev)
    trc("node-prev", value::from_permanent_ptr(ptr->prev->_private));
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

void node::prop_parent(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    if (!ptr->parent)
      data = object();
    else
      data = create(ptr->parent);
    break;
  case property_set:
    if (data.is_void() || data.is_null()) {
      xmlUnlinkNode(ptr);
      data = object();
    } else if (!data.is_object()) {
      data = value();
    } else {
      xmlNodePtr parent = c_from_js(data.get_object());
      if (!parent) {
        data = value();
        break;
      }
      if (ptr->parent)
        xmlUnlinkNode(ptr);
      xmlAddChild(parent, ptr);
    }
    break;
  default: break;
  }
}

void node::prop_next(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    if (!ptr->next)
      data = object();
    else
      data = create(ptr->next);
    break;
  case property_set:
    if (data.is_void() || data.is_null()) {
      if (ptr->parent)
        ptr->parent->last = ptr;
      ptr->next = 0;
      data = object();
    } else if (!data.is_object()) {
      data = value();
    } else {
      xmlNodePtr next = c_from_js(data.get_object());
      if (!next) {
        data = value();
        break;
      }
      ptr->next = next;
      if (next->prev) {
        if (next->prev->parent)
          next->prev->parent->last = next->prev;
        next->prev->next = 0;
      }
      next->prev = ptr;
      while (next) {
        next->parent = ptr->parent;
        if (!next->next && next->parent)
          next->parent->last = next;
        next = next->next;
      }
      xmlSetListDoc(ptr->next, ptr->doc);
    }
    break;
  default: break;
  }
}

void node::prop_prev(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    if (!ptr->prev)
      data = object();
    else
      data = create(ptr->prev);
    break;
  case property_set:
    if (data.is_void() || data.is_null()) {
      if (ptr->parent)
        ptr->parent->children = ptr;
      ptr->prev = 0;
      data = object();
    } else if (!data.is_object()) {
      data = value();
    } else {
      xmlNodePtr prev = c_from_js(data.get_object());
      if (!prev) {
        data = value();
        break;
      }
      ptr->prev = prev;
      if (prev->next) {
        if (prev->next->parent)
          prev->next->parent->children = prev->next;
        prev->next->prev = 0;
      }
      prev->next = ptr;
      while (prev) {
        xmlSetTreeDoc(prev, ptr->doc);
        prev->parent = ptr->parent;
        if (!prev->prev && prev->parent) 
          prev->parent->children = prev;
        prev = prev->prev;
      }
    }
    break;
  default: break;
  }
}

void node::prop_document(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  if (!ptr->doc)
    data = object();
  else
    data = create(xmlNodePtr(ptr->doc));
}

void node::prop_type(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  switch (ptr->type) {
  case XML_ELEMENT_NODE:
    data = string("ELEMENT");
    break;
  case XML_ATTRIBUTE_NODE:
    data = string("ATTRIBUTE");
    break;
  case XML_TEXT_NODE:
    data = string("TEXT");
    break;
  case XML_CDATA_SECTION_NODE:
    data = string("CDATA-SECTION");
    break;
  case XML_ENTITY_REF_NODE:
    data = string("ENTITY-REFERENCE");
    break;
  case XML_ENTITY_NODE:
    data = string("ENTITY");
    break;
  case XML_PI_NODE:
    data = string("PI");
    break;
  case XML_COMMENT_NODE:
    data = string("COMMENT");
    break;
  case XML_DOCUMENT_NODE:
    data = string("DOCUMENT");
    break;
  case XML_DOCUMENT_TYPE_NODE:
    data = string("DOCUMENT-TYPE");
    break;
  case XML_DOCUMENT_FRAG_NODE:
    data = string("DOCUMENT-FRAGMENT");
    break;
  case XML_NOTATION_NODE:
    data = string("NOTATION");
    break;
  case XML_HTML_DOCUMENT_NODE:
    data = string("HTML-DOCUMENT");
    break;
  case XML_DTD_NODE:
    data = string("DTD");
    break;
  case XML_ELEMENT_DECL:
    data = string("ELEMENT-DECLARATION");
    break;
  case XML_ATTRIBUTE_DECL:
    data = string("ATTRIBUTE-DECLARATION");
    break;
  case XML_ENTITY_DECL:
    data = string("ENTITY-DECLARATION");
    break;
  case XML_NAMESPACE_DECL:
    data = string("NAMESPACE-DECLARATION");
    break;
  case XML_XINCLUDE_START:
    data = string("XINCLUDE-START");
    break;
  case XML_XINCLUDE_END:
    data = string("XINCLUDE-END");
    break;
  default:
    data = string("OTHER");
    break;
  }
}

object node::copy(bool recursive) {
  xmlNodePtr copy = xmlCopyNode(ptr, recursive);

  if (!copy)
    throw exception("Could not copy XML node");

  return create(copy);
}

