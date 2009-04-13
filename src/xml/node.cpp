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
#include "flusspferd/xml/html_document.hpp"
#include "flusspferd/xml/text.hpp"
#include "flusspferd/xml/namespace.hpp"
#include "flusspferd/xml/reference.hpp"
#include "flusspferd/xml/processing_instruction.hpp"
#include "flusspferd/xml/attribute.hpp"
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
  case XML_HTML_DOCUMENT_NODE:
    return create_native_object<html_document>(object(), htmlDocPtr(ptr));
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
  case XML_ATTRIBUTE_NODE:
    return create_native_object<attribute_>(object(), xmlAttrPtr(ptr));
  default:
    return create_native_object<node>(object(), ptr);
  }
}

void node::create_all_children(
    xmlNodePtr ptr, bool children, bool properties)
{
  if (ptr->type == XML_ENTITY_REF_NODE)
    return;

  if (ptr->type == XML_ELEMENT_NODE || ptr->type == XML_ATTRIBUTE_NODE) {
    if (ptr->ns)
      namespace_::create(ptr->ns);
  }

  if (ptr->type == XML_ELEMENT_NODE) {
    xmlNsPtr ns = ptr->nsDef;
    while (ns) {
      namespace_::create(ns);
      ns = ns->next;
    }
  }

  if (children) {
    for (xmlNodePtr child = ptr->children; child; child = child->next)
      create(child);
  }

  if (properties && ptr->type == XML_ELEMENT_NODE) {
    for (xmlAttrPtr prop = ptr->properties; prop; prop = prop->next)
      create(xmlNodePtr(prop));
  }
}

xmlNodePtr node::c_from_js(object const &obj) {
  if (obj.is_null())
    return 0;

  try {
    return flusspferd::get_native<node>(obj).c_obj();
  } catch (std::exception&) {
    return 0;
  }
}

node::node(object const &obj, xmlNodePtr ptr)
  : native_object_base(obj), ptr(ptr)
{
  ptr->_private = static_cast<object*>(this);
  create_all_children(ptr);

  init();
}

node::node(object const &obj, call_context &x)
  : native_object_base(obj)
{
  local_root_scope scope;

  xmlDocPtr doc = document::c_from_js(x.arg[0].to_object());

  std::size_t offset = !doc ? 0 : 1;

  value name_v(x.arg[offset + 0]);
  if (!name_v.is_string())
    throw exception("Could not create XML node: name has to be a string");
  xmlChar const *name = (xmlChar const *) name_v.to_string().c_str();

  xmlChar *prefix = 0;

  if (xmlChar const *colon = xmlStrchr(name, ':')) {
    std::size_t len = colon - name;
    prefix = xmlStrsub(name, 0, len);
    name += len + 1;
  }

  try {
    value ns_v(x.arg[offset + 1]);

    xmlNsPtr ns = 0;
    if (ns_v.is_object()) {
      ns = namespace_::c_from_js(ns_v.get_object());
      if (prefix) {
        if (ns->prefix) xmlFree((xmlChar*) ns->prefix);
        ns->prefix = prefix;
        prefix = 0; // don't free
      }
    }

    ptr = xmlNewDocNode(doc, ns, name, 0);

    if (!ptr)
      throw exception("Could not create XML node");

    ptr->_private = static_cast<object*>(this);

    if (ns_v.is_string()) {
      ns = xmlNewNs(ptr, (xmlChar const *) ns_v.get_string().c_str(), prefix);
      namespace_::create(ns);
      ptr->ns = ns;
    }
  } catch (...) {
    if (prefix) xmlFree(prefix);
    throw;
  }

  if (prefix) xmlFree(prefix);

  init();
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

    if (ptr->type == XML_ELEMENT_NODE) {
      ptr->properties = 0;
      ptr->nsDef = 0;
    }

    if (ptr->type == XML_ELEMENT_NODE || ptr->type == XML_ATTRIBUTE_NODE)
      ptr->ns = 0;

    xmlFreeNode(ptr);
  }
}

void node::init() {
//FIXME
#if 0
  define_native_property("lang", RW, &node::prop_lang);
  define_native_property("content", RW, &node::prop_content);
  define_native_property("parent", RW, &node::prop_parent);
  define_native_property("nextSibling", RW, &node::prop_next);
  define_native_property("previousSibling", RW, &node::prop_prev);
  define_native_property("firstChild", RW, &node::prop_first_child);
  define_native_property("lastChild", RO, &node::prop_last_child);
  define_native_property("firstSibling", RO, &node::prop_first_sibling);
  define_native_property("lastSibling", RO, &node::prop_last_sibling);
  define_native_property("document", RO, &node::prop_document);

  if (ptr->type  == XML_ELEMENT_NODE || ptr->type == XML_ATTRIBUTE_NODE) {
    define_native_property("namespace", RW, &node::prop_namespace);
    define_native_property("namespaces", RO, &node::prop_namespaces);
  }

  define_native_property("firstAttribute", RW, &node::prop_first_attr);
#endif
}

object node::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "copy", &node::copy);
  create_native_method(proto, "unlink", &node::unlink);
  create_native_method(proto, "purge", &node::purge);
  create_native_method(proto, "addContent", &node::add_content);
  create_native_method(proto, "addChild", &node::add_child);
  create_native_method(proto, "addChildList", &node::add_child_list);
  create_native_method(proto, "addNode", &node::add_node);
  create_native_method(proto, "addNamespace", &node::add_namespace);
  create_native_method(proto, "addAttribute", &node::add_attribute);
  create_native_method(proto, "setAttribute", &node::set_attribute);
  create_native_method(proto, "unsetAttribute", &node::unset_attribute);
  create_native_method(proto, "findAttribute", &node::find_attribute);
  create_native_method(proto, "getAttribute", &node::get_attribute);
  create_native_method(proto, "searchNamespaceByPrefix",
                       &node::search_namespace_by_prefix);
  create_native_method(proto, "searchNamespaceByURI",
                       &node::search_namespace_by_uri);
  create_native_method(proto, "toString", &node::to_string);

  proto.define_property(
    "name", value(),
    property_attributes(
      permanent_shared_property,
      create_native_method(object(), "", &node::get_name),
      create_native_method(object(), "", &node::set_name)));

  proto.define_property(
    "type", value(),
    property_attributes(
      permanent_shared_property | read_only_property,
      create_native_method(object(), "", &node::get_type)));

  return proto;
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

  if (ptr->type == XML_ELEMENT_NODE && ptr->properties)
    trc("node-properties", *static_cast<object*>(ptr->properties->_private));

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

void node::set_name(std::string const &s) {
  xmlNodeSetName(ptr, (xmlChar const *) s.c_str());
}

std::string node::get_name() {
  if (!ptr->name)
    return std::string();
  else
    return std::string((char const *) ptr->name);
}

void node::prop_lang(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    xmlNodeSetLang(ptr, (xmlChar const *) data.to_string().c_str());
    create_all_children(ptr, false, true);
    // !! fall thru !!
  case property_get:
    {
      xmlChar const *lang = xmlNodeGetLang(ptr);
      if (!lang)
        data = value();
      else
        data = string((char const *) lang);
    }
    break;
  default: break;
  };
}

void node::prop_content(property_mode mode, value &data) {
  xmlChar *content;
  switch (mode) {
  case property_set:
    xmlNodeSetContent(ptr, 0);
    if (!data.is_undefined() && !data.is_null())
      xmlNodeAddContent(ptr, (xmlChar const *) data.to_string().c_str());
    create_all_children(ptr, true, false);
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
  case property_set:
    if (data.is_undefined() || data.is_null()) {
      if (ptr->parent) {
        if (ptr->type == XML_ATTRIBUTE_NODE) {
          if (ptr->parent->type == XML_ELEMENT_NODE && !ptr->prev)
            ptr->parent->properties = 0;
        } else {
          ptr->parent->last = ptr->prev;
          if (!ptr->prev)
            ptr->parent->children = 0;
        }
        
        if (ptr->prev) {
          ptr->prev->next = 0;
          ptr->prev = 0;
        }
        ptr->parent = 0;
        xmlSetListDoc(ptr, 0);
      }
    } else if (xmlNodePtr parent = c_from_js(data.to_object())) {
      xmlNodePtr old_parent = ptr->parent;
      if (ptr->prev)
        ptr->prev->next = 0;
      ptr->prev = 0;
      if (ptr->type == XML_ATTRIBUTE_NODE) {
        if (parent->type == XML_ELEMENT_NODE) {
          ptr->parent = parent;
          xmlAttrPtr ptr = xmlAttrPtr(this->ptr);
          if (xmlAttrPtr last = parent->properties) {
            while (last->next)
              last = last->next;
            last->next = ptr;
            ptr->prev = last;
          } else {
            parent->properties = ptr;
            ptr->prev = 0;
          }
          while (ptr->next) {
            ptr = ptr->next;
            ptr->parent = parent;
          }
        }
      } else {
        ptr->parent = parent;
        if (old_parent) {
          old_parent->last = ptr->prev;
          if (!ptr->prev) 
            old_parent->children = 0;
        }
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
      }
      xmlSetListDoc(ptr, parent->doc);
    }
    // !! fall thru !!
  case property_get:
    data = create(ptr->parent);
    break;
  default: break;
  }
}

void node::prop_next(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    if (data.is_undefined() || data.is_null()) {
      if (ptr->parent)
        if (ptr->type != XML_ATTRIBUTE_NODE)
          ptr->parent->last = ptr;
      ptr->next = 0;
    } else if (xmlNodePtr next = c_from_js(data.to_object())) {
      ptr->next = next;
      if (xmlNodePtr np = next->prev) {
        if (np->parent)
          if (next->type != XML_ATTRIBUTE_NODE)
            np->parent->last = np;
        np->next = 0;
      } else if (next->parent) {
        if (next->type == XML_ATTRIBUTE_NODE) {
          if (next->parent->type == XML_ELEMENT_NODE)
            next->parent->properties = 0;
        } else {
          next->parent->children = 0;
          next->parent->last = 0;
        }
      }
      next->prev = ptr;
      if (next->type != XML_ATTRIBUTE_NODE) {
        while (next) {
          next->parent = ptr->parent;
          if (!next->next && next->parent)
            next->parent->last = next;
          next = next->next;
        }
      }
      xmlSetListDoc(ptr->next, ptr->doc);
    }
    // !! fall thru !!
  case property_get:
    data = create(ptr->next);
    break;
  default: break;
  }
}

void node::prop_prev(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    if (data.is_undefined() || data.is_null()) {
      if (ptr->parent) {
        if (ptr->type == XML_ATTRIBUTE_NODE) {
          if (ptr->parent->type == XML_ELEMENT_NODE)
            ptr->parent->properties = xmlAttrPtr(ptr);
        } else {
          ptr->parent->children = ptr;
        }
      }
      ptr->prev = 0;
    } else if (xmlNodePtr prev = c_from_js(data.get_object())) {
      ptr->prev = prev;
      if (xmlNodePtr pn = prev->next) {
        if (pn->parent) {
          if (pn->type == XML_ATTRIBUTE_NODE) {
            if (ptr->parent->type == XML_ELEMENT_NODE)
              ptr->parent->properties = xmlAttrPtr(pn);
          } else {
            pn->parent->children = pn;
          }
        }
        pn->prev = 0;
      } else if (prev->parent) {
        if (prev->type != XML_ATTRIBUTE_NODE) {
          prev->parent->children = 0;
          prev->parent->last = 0;
        } else {
          if (ptr->parent->type == XML_ELEMENT_NODE)
            prev->parent->properties = 0;
        }
      }
      prev->next = ptr;
      while (prev) {
        xmlSetTreeDoc(prev, ptr->doc);
        prev->parent = ptr->parent;
        if (!prev->prev && prev->parent)  {
          if (prev->type == XML_ATTRIBUTE_NODE) {
            if (prev->parent->type == XML_ELEMENT_NODE)
              prev->parent->properties = xmlAttrPtr(prev);
          } else {
            prev->parent->children = prev;
          }
        }
        prev = prev->prev;
      }
    }
    // !! fall thru !!
  case property_get:
    data = create(ptr->prev);
    break;
  default: break;
  }
}

void node::prop_first_child(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    if (data.is_undefined() || data.is_null()) {
      xmlNodePtr old = ptr->children;
      while (old) {
        old->parent = 0;
        old = old->next;
      }
      ptr->children = 0;
      ptr->last = 0;
      data = object();
    } else if (xmlNodePtr child = c_from_js(data.get_object())) {
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
    // !! fall thru !!
  case property_get:
    data = create(ptr->type != XML_ENTITY_REF_NODE ? ptr->children : 0);
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

std::string node::get_type() {
  switch (ptr->type) {
  case XML_ELEMENT_NODE:       return "ELEMENT";               break;
  case XML_ATTRIBUTE_NODE:     return "ATTRIBUTE";             break;
  case XML_TEXT_NODE:          return "TEXT";                  break;
  case XML_CDATA_SECTION_NODE: return "CDATA-SECTION";         break;
  case XML_ENTITY_REF_NODE:    return "ENTITY-REFERENCE";      break;
  case XML_ENTITY_NODE:        return "ENTITY";                break;
  case XML_PI_NODE:            return "PI";                    break;
  case XML_COMMENT_NODE:       return "COMMENT";               break;
  case XML_DOCUMENT_NODE:      return "DOCUMENT";              break;
  case XML_DOCUMENT_TYPE_NODE: return "DOCUMENT-TYPE";         break;
  case XML_DOCUMENT_FRAG_NODE: return "DOCUMENT-FRAGMENT";     break;
  case XML_NOTATION_NODE:      return "NOTATION";              break;
  case XML_HTML_DOCUMENT_NODE: return "HTML-DOCUMENT";         break;
  case XML_DTD_NODE:           return "DTD";                   break;
  case XML_ELEMENT_DECL:       return "ELEMENT-DECLARATION";   break;
  case XML_ATTRIBUTE_DECL:     return "ATTRIBUTE-DECLARATION"; break;
  case XML_ENTITY_DECL:        return "ENTITY-DECLARATION";    break;
  case XML_NAMESPACE_DECL:     return "NAMESPACE-DECLARATION"; break;
  case XML_XINCLUDE_START:     return "XINCLUDE-START";        break;
  case XML_XINCLUDE_END:       return "XINCLUDE-END";          break;
  default:                     return "OTHER";                 break;
  }
}

void node::prop_namespace(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    if (data.is_undefined() || data.is_null()) {
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
    // !! fall thru !!
  case property_get:
    data = namespace_::create(ptr->ns);
    break;
  default: break;
  }
}

void node::prop_namespaces(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  xmlNsPtr *nsList = xmlGetNsList(ptr->doc, ptr);

  try {
    object array = create_array();
    data = array;

    if (!nsList)
      return;

    for (xmlNsPtr *p = nsList; *p; ++p) {
      array.call("push", namespace_::create(*p));
    }
  } catch (...) {
    if (nsList) xmlFree(nsList);
    throw;
  }
  if (nsList) xmlFree(nsList);
}

void node::prop_first_attr(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
    if (ptr->type == XML_ELEMENT_NODE) {
      if (data.is_undefined_or_null()) {
        ptr->properties = 0;
      } else if (data.is_object()) {
        xmlNodePtr attr = c_from_js(data.get_object());
        if (attr && attr->type == XML_ATTRIBUTE_NODE) {
          ptr->properties = xmlAttrPtr(attr);
          attr->parent = ptr;
        }
      }
    }
    // !! fall thru !!
  case property_get:
    data = create(ptr->type == XML_ELEMENT_NODE
                  ? xmlNodePtr(ptr->properties)
                  : 0);
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
  create_all_children(ptr, true, false);
}

void node::add_node(call_context &x) {
  if (ptr->doc) {
    arguments arg;
    arg.push_root(create(xmlNodePtr(ptr->doc)));

    // name
    arg.push_back(x.arg[0]);

    // namespace
    if (x.arg[1].is_string()) {
      value ns = call("searchNamespaceByURI", x.arg[1]);
      if (ns.is_object() && !ns.is_null())
        x.arg[1] = ns;
    }
    arg.push_back(x.arg[1]);
    x.arg = arg;
  }
  object obj = create_native_object<node>(object(), boost::ref(x));
  obj.set_property("parent", *this);
  x.result = obj;
}

void node::add_namespace(call_context &x) {
  value ns = call("searchNamespaceByURI", x.arg[0]);
  if (ns.is_object() && !ns.is_null()) {
    x.result = ns;
    return;
  }

  arguments arg;
  arg.push_back(*this);
  arg.push_back(x.arg[0]); // href
  arg.push_back(x.arg[1]); // prefix
  x.arg = arg;
  object obj = create_native_object<namespace_>(object(), boost::ref(x));
  x.result = obj;
}

void node::add_attribute(call_context &x) {
  arguments arg;
  arg.push_back(*this);
  for (arguments::iterator it = x.arg.begin(); it != x.arg.end(); ++it)
    arg.push_root(*it);
  x.arg = arg;
  x.result = create_native_object<attribute_>(object(), boost::ref(x));
  purge();
}

void node::set_attribute(call_context &x) {
  if (x.arg.size() < 3)
    throw exception("Could not set XML attribute: too few parameters");

  if (!x.arg[1].is_object())
    throw exception("Could not set XML attribute: "
                    "namespace has to be an object");

  find_attribute(x);

  if (x.result.is_null()) {
    add_attribute(x);
  } else {
    object o = x.result.to_object();
    o.set_property("content", x.arg[2]);
  }
}

void node::unset_attribute(call_context &x) {
  find_attribute(x);

  if (!x.result.is_null() && x.result.is_object())
    x.result.get_object().call("unlink");

  x.result = value();
}

void node::find_attribute(call_context &x) {
  local_root_scope scope;

  if (!x.arg[0].is_string())
    throw exception("Could not find XML attribute: name has to be a string");

  xmlChar const *name = (xmlChar const *) x.arg[0].get_string().c_str();

  xmlChar const *ns_href = 0;
  if (x.arg[1].is_string()) {
    ns_href = (xmlChar const *) x.arg[1].get_string().c_str();
  } else if (!x.arg[1].is_undefined_or_null()) {
    xmlNsPtr ns = namespace_::c_from_js(x.arg[1].to_object());
    if (!ns)
      throw exception("Could not find XML attribute: "
                      "no valid namespace specified");
    ns_href = ns->href;
  }

  xmlAttrPtr prop = xmlHasNsProp(ptr, name, ns_href);

  x.result = create(xmlNodePtr(prop));
}

void node::get_attribute(call_context &x) {
  find_attribute(x);

  if (x.result.is_object() && !x.result.is_null()) {
    object o = x.result.get_object();
    x.result = o.get_property("content");
  } else {
    x.result = value();
  }
}

void node::add_child(node &nd) {
  nd.unlink();
  add_child_list(nd);
}

void node::add_child_list(node &nd) {
  nd.set_property("parent", *this);
  purge();
}

string node::to_string() {
  local_root_scope scope;

  string type = get_property("type").to_string();
  string name = get_property("name").to_string();

  return string::concat(string::concat(type, ":"), name);
}

object node::search_namespace_by_prefix(value const &prefix_) {
  local_root_scope scope;
  xmlChar const *prefix = 0; 
  if (!prefix_.is_string() && !prefix_.is_undefined() && !prefix_.is_null())
    throw exception("Could not search for non-string namespace prefix");
  if (prefix_.is_string())
    prefix = (xmlChar const *) prefix_.get_string().c_str();
  xmlNsPtr ns = xmlSearchNs(ptr->doc, ptr, prefix);
  return namespace_::create(ns);
}

object node::search_namespace_by_uri(string const &uri_) {
  xmlChar const *uri = (xmlChar const *) uri_.c_str();
  xmlNsPtr ns = xmlSearchNsByHref(ptr->doc, ptr, uri);
  return namespace_::create(ns);
}

void node::purge() {
  if (ptr->type == XML_ELEMENT_NODE) {
    for (xmlAttrPtr prop = ptr->properties; prop; prop = prop->next) {
      for (xmlAttrPtr prop2 = prop->next; prop2; prop2 = prop2 ->next) {
        xmlChar const *href1 = prop->ns ? prop->ns->href : 0;
        xmlChar const *href2 = prop2->ns ? prop2->ns->href : 0;
        if (bool(href1) != bool(href2))
          continue;
        if (href1 && href1 != href2 && xmlStrcmp(href1, href2))
          continue;
        if (prop->name == 0 || prop2->name == 0)
          continue;
        if (xmlStrcmp(prop->name, prop2->name) == 0)
          xmlUnlinkNode(xmlNodePtr(prop));
      }
    }
  }

  if (!ptr->children)
    return;

  for (xmlNodePtr child = ptr->children->next; child;) {
    xmlNodePtr next = child->next;
    if (child->type == XML_TEXT_NODE && child->prev &&
        child->prev->type == XML_TEXT_NODE)
    {
      xmlNodeAddContent(child->prev, child->content);
      xmlUnlinkNode(child);
    }
    child = next;
  }
}
