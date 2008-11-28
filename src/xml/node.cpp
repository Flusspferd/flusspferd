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
#include "flusspferd/xml/text.hpp"
#include "flusspferd/xml/namespace.hpp"
#include "flusspferd/xml/reference.hpp"
#include "flusspferd/xml/processing_instruction.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/local_root_scope.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

object node::create(xmlNodePtr ptr) {
  if (!ptr)
    return object();

  if (ptr->_private)
    return *static_cast<object*>(ptr->_private);

  switch (ptr->type) {
  case XML_DOCUMENT_NODE:
    return create_native_object<document>(object(), xmlDocPtr(ptr));
  case XML_TEXT_NODE:
    return create_native_object<text>(object(), ptr);
  case XML_COMMENT_NODE:
    return create_native_object<comment>(object(), ptr);
  case XML_CDATA_SECTION_NODE:
    return create_native_object<cdata_section>(object(), ptr);
  case XML_ENTITY_REF_NODE:
    return create_native_object<reference_>(object(), ptr);
  case XML_PI_NODE:
    return create_native_object<processing_instruction>(object(), ptr);
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
  ptr->_private = static_cast<object*>(this);
}

node::node(call_context &x) {
  local_root_scope scope;

  xmlDocPtr doc = document::c_from_js(x.arg[0].to_object());

  std::size_t offset = !doc ? 0 : 1;

  value name_v(x.arg[offset + 0]);
  if (!name_v.is_string())
    throw exception("Could not create XML node: name has to be a string");
  xmlChar const *name = (xmlChar const *) name_v.to_string().c_str();

  value ns_v(x.arg[offset + 1]);

  xmlNsPtr ns = 0;
  if (ns_v.is_object()) {
    ns = namespace_::c_from_js(ns_v.get_object());
  }

  ptr = xmlNewDocNode(doc, ns, name, 0);

  if (!ptr)
    throw exception("Could not create XML node");

  ptr->_private = static_cast<object*>(this);

  if (ns_v.is_string()) {
    ns = xmlNewNs(ptr, (xmlChar const *) ns_v.get_string().c_str(), 0);
    namespace_::create(ns);
    ptr->ns = ns;
  }
}

node::~node() {
  if (ptr && ptr->_private == static_cast<object*>(this)) {
    xmlUnlinkNode(ptr);

    xmlNodePtr x = ptr->children;
    while (x) {
      x->parent = 0;
      x = x->next;
    }
    ptr->children = 0;
    ptr->last = 0;

    ptr->ns = 0;
    ptr->nsDef = 0;

    xmlFreeNode(ptr);
  }
}

void node::post_initialize() {
  register_native_method("copy", &node::copy);
  register_native_method("unlink", &node::unlink);
  register_native_method("addContent", &node::add_content);
  register_native_method("addChild", &node::add_child);
  register_native_method("toString", &node::to_string);

  unsigned const RW = permanent_property | dont_enumerate;
  unsigned const RO = permanent_property | dont_enumerate | read_only_property;

  define_native_property("name", RW, &node::prop_name);
  define_native_property("lang", RW, &node::prop_lang);
  define_native_property("content", RW, &node::prop_content);
  define_native_property("parent", RW, &node::prop_parent);
  define_native_property("next", RW, &node::prop_next);
  define_native_property("nextSibling", RW, &node::prop_next);
  define_native_property("prev", RW, &node::prop_prev);
  define_native_property("previousSibling", RW, &node::prop_prev);
  define_native_property("firstChild", RW, &node::prop_first_child);
  define_native_property("lastChild", RO, &node::prop_last_child);
  define_native_property("firstSibling", RO, &node::prop_first_sibling);
  define_native_property("lastSibling", RO, &node::prop_last_sibling);
  define_native_property("document", RO, &node::prop_document);
  define_native_property("type", RO, &node::prop_type);

  if (ptr->type  == XML_ELEMENT_NODE)
    define_native_property("namespace", RW, &node::prop_namespace);
}

object node::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "copy", 1);
  create_native_method(proto, "unlink", 0);
  create_native_method(proto, "addContent", 1);
  create_native_method(proto, "addChild", 1);
  create_native_method(proto, "toString", 0);

  return proto;
}

char const *node::class_info::constructor_name() {
  return "Node";
}

std::size_t node::class_info::constructor_arity() {
  return 2;
}

static void trace_children(tracer &trc, xmlNodePtr ptr) {
  while (ptr) {
    trc("node-children", *static_cast<object*>(ptr->_private));
    ptr = ptr->next;
  }
}

static void trace_parents(tracer &trc, xmlNodePtr ptr) {
  while (ptr) {
    trc("node-parent", *static_cast<object*>(ptr->_private));
    ptr = ptr->parent;
  }
}

void node::trace(tracer &trc) {
  trc("node-self", *static_cast<object*>(ptr->_private));

  if (ptr->type != XML_ENTITY_REF_NODE)
    trace_children(trc, ptr->children);

  trace_parents(trc, ptr->parent);

  if (ptr->doc)
    trc("node-doc", *static_cast<object*>(ptr->doc->_private));

  if (ptr->next)
    trc("node-next", *static_cast<object*>(ptr->next->_private));

  if (ptr->prev)
    trc("node-prev", *static_cast<object*>(ptr->prev->_private));

  if (ptr->type == XML_ELEMENT_NODE || ptr->type == XML_ATTRIBUTE_NODE) {
    if (ptr->ns)
      trc("node-ns", *static_cast<object*>(ptr->ns->_private));
  }

  if (ptr->type == XML_ELEMENT_NODE) {
    xmlNsPtr nsDef = ptr->nsDef;
    while (nsDef) {
      trc("node-nsDef", *static_cast<object*>(nsDef->_private));
      nsDef = nsDef->next;
    }
  }
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

void node::prop_content(property_mode mode, value &data) {
  xmlChar *content;
  switch (mode) {
  case property_set:
    xmlNodeSetContent(ptr, (xmlChar const *) "");
    if (!data.is_void() && !data.is_null())
      xmlNodeAddContent(ptr, (xmlChar const *) data.to_string().c_str());
    for (xmlNodePtr child = ptr->children; child; child = child->next)
      create(child);
    // !! fall thru !!
  case property_get:
    content = xmlNodeGetContent(ptr);
    if (!content) {
      data = value();
    } else {
      data = string((char const *) content);
      xmlFree(content);
    }
    break;
  default: break;
  }
}

void node::prop_parent(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    data = create(ptr->parent);
    break;
  case property_set:
    if (data.is_void() || data.is_null()) {
      if (ptr->parent) {
        ptr->parent->last = ptr->prev;
        if (!ptr->prev) {
          ptr->parent->children = 0;
        } else {
          ptr->prev->next = 0;
          ptr->prev = 0;
        }
        ptr->parent = 0;
        xmlSetListDoc(ptr, 0);
      }
      data = object();
    } else if (!data.is_object()) {
      data = value();
    } else {
      xmlNodePtr parent = c_from_js(data.get_object());
      if (!parent) {
        data = value();
        break;
      }
      if (ptr->parent) {
        ptr->parent->last = ptr->prev;
        if (!ptr->prev) 
          ptr->parent->children = 0;
      }
      if (ptr->prev)
        ptr->prev->next = 0;
      ptr->parent = parent;
      if (parent->last) {
        ptr->prev = parent->last;
        parent->last->next = ptr;
      } else {
        ptr->prev = 0;
        parent->children = ptr;
        parent->last = ptr;
      }
      while (parent->last->next) {
        parent->last = parent->last->next;
        parent->last->parent = parent;
      }
      xmlSetListDoc(ptr, parent->doc);
    }
    break;
  default: break;
  }
}

void node::prop_next(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
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
      } else if (next->parent) {
        next->parent->children = 0;
        next->parent->last = 0;
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
      } else if (prev->parent) {
        prev->parent->children = 0;
        prev->parent->last = 0;
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

void node::prop_first_child(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    data = create(ptr->type != XML_ENTITY_REF_NODE ? ptr->children : 0);
    break;
  case property_set:
    if (data.is_void() || data.is_null()) {
      xmlNodePtr old = ptr->children;
      while (old) {
        old->parent = 0;
        old = old->next;
      }
      ptr->children = 0;
      ptr->last = 0;
      data = object();
    } else if (!data.is_object()) {
      data = value();
    } else {
      xmlNodePtr child = c_from_js(data.get_object());
      if (!child) {
        data = value();
        break;
      }
      xmlNodePtr old = ptr->children;
      while (old) {
        old->parent = 0;
        old = old->next;
      }
      ptr->children = child;
      child->parent = ptr;
      while (child->next) {
        child = child->next;
        child->parent = ptr;
      }
      ptr->last = child;
    }
    break;
  default: break;
  }
}

void node::prop_last_child(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  data = create(ptr->type != XML_ENTITY_REF_NODE ? ptr->last : 0);
}

void node::prop_first_sibling(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  if (ptr->parent) {
    data = create(ptr->parent->children);
    return;
  }

  xmlNodePtr ptr = this->ptr;
  while (ptr->prev)
    ptr = ptr->prev;
  data = create(ptr);
}

void node::prop_last_sibling(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  if (ptr->parent) {
    data = create(ptr->parent->last);
    return;
  }

  xmlNodePtr ptr = this->ptr;
  while (ptr->next)
    ptr = ptr->next;
  data = create(ptr);
}

void node::prop_document(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  data = create(xmlNodePtr(ptr->doc));
}

void node::prop_type(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  switch (ptr->type) {
  case XML_ELEMENT_NODE:       data = string("ELEMENT");               break;
  case XML_ATTRIBUTE_NODE:     data = string("ATTRIBUTE");             break;
  case XML_TEXT_NODE:          data = string("TEXT");                  break;
  case XML_CDATA_SECTION_NODE: data = string("CDATA-SECTION");         break;
  case XML_ENTITY_REF_NODE:    data = string("ENTITY-REFERENCE");      break;
  case XML_ENTITY_NODE:        data = string("ENTITY");                break;
  case XML_PI_NODE:            data = string("PI");                    break;
  case XML_COMMENT_NODE:       data = string("COMMENT");               break;
  case XML_DOCUMENT_NODE:      data = string("DOCUMENT");              break;
  case XML_DOCUMENT_TYPE_NODE: data = string("DOCUMENT-TYPE");         break;
  case XML_DOCUMENT_FRAG_NODE: data = string("DOCUMENT-FRAGMENT");     break;
  case XML_NOTATION_NODE:      data = string("NOTATION");              break;
  case XML_HTML_DOCUMENT_NODE: data = string("HTML-DOCUMENT");         break;
  case XML_DTD_NODE:           data = string("DTD");                   break;
  case XML_ELEMENT_DECL:       data = string("ELEMENT-DECLARATION");   break;
  case XML_ATTRIBUTE_DECL:     data = string("ATTRIBUTE-DECLARATION"); break;
  case XML_ENTITY_DECL:        data = string("ENTITY-DECLARATION");    break;
  case XML_NAMESPACE_DECL:     data = string("NAMESPACE-DECLARATION"); break;
  case XML_XINCLUDE_START:     data = string("XINCLUDE-START");        break;
  case XML_XINCLUDE_END:       data = string("XINCLUDE-END");          break;
  default:                     data = string("OTHER");                 break;
  }
}

void node::prop_namespace(property_mode mode, value &data) {
  switch (mode) {
  case property_get:
    data = namespace_::create(ptr->ns);
    break;
  case property_set:
    if (data.is_void() || data.is_null()) {
      ptr->ns->context = 0;
      ptr->ns = 0;
      data = object();
    } else if (data.is_object()) {
      xmlNsPtr ns = namespace_::c_from_js(data.get_object());
      if (!ns) {
        data = value();
        break;
      }
      ptr->ns = ns;
    } else {
      data = value();
    }
    break;
  default: break;
  }
}

object node::copy(bool recursive) {
  xmlNodePtr copy = xmlCopyNode(ptr, recursive);
  if (!copy)
    throw exception("Could not copy XML node");
  return create(copy);
}

void node::unlink() {
  xmlUnlinkNode(ptr);
}

void node::add_content(string const &content) {
  xmlChar const *text = (xmlChar const *) content.c_str();
  xmlNodeAddContent(ptr, text);
  for (xmlNodePtr child = ptr->children; child; child = child->next)
    create(child);
}

void node::add_child(node &nd) {
  nd.unlink();
  nd.ptr = xmlAddChild(ptr, nd.ptr);
  if (!nd.ptr->_private)
    nd.ptr->_private = static_cast<object*>(&nd);
}

string node::to_string() {
  local_root_scope scope;

  string type = get_property("type").to_string();
  string name = get_property("name").to_string();

  return string::concat(string::concat(type, ":"), name);
}
