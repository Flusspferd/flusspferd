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

#include "flusspferd/xml/document.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/tracer.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

xmlDocPtr document::c_from_js(object const &obj) {
  if (!obj.is_valid())
    return 0;
  try {
    xml::document *p =
      dynamic_cast<xml::document*>(native_object_base::get_native(obj));
    if (!p)
      return 0;
    return p->c_obj();
  } catch (std::exception&) {
    return 0;
  }
}

document::document(object const &obj, xmlDocPtr ptr)
  : node(obj, xmlNodePtr(ptr))
{
  init();
}

static xmlDocPtr new_doc(call_context &) {
  xmlDocPtr ptr = xmlNewDoc((xmlChar const *) "1.0");
  if (!ptr)
    throw exception("Could not create empty XML document");
  return ptr;
}

document::document(object const &obj, call_context &x)
  : node(obj, xmlNodePtr(new_doc(x)))
{
  init();
}

document::~document() {
  if (c_obj()->_private == static_cast<object*>(this)) {
    xmlNodePtr x = c_obj()->children;
    while (x) {
      x->parent = 0;
      x = x->next;
    }
    c_obj()->children = 0;
    c_obj()->last = 0;
    c_obj()->oldNs = 0;
    xmlFreeDoc(c_obj());
    set_c_obj(0);
  }
}

void document::trace(tracer &trc) {
  node::trace(trc);
  if (c_obj()->oldNs)
    trc("doc-oldns", *static_cast<object*>(c_obj()->oldNs->_private));
}

void document::init() {
  register_native_method("dump", &document::dump);
  register_native_method("copy", &document::copy);
  register_native_method("toString", &document::to_string);

  unsigned const RW = permanent_shared_property;
  unsigned const RO = RW |read_only_property;

  define_native_property("rootElement", RW, &document::prop_root_element);
  define_native_property("xmlNamespace", RO, &document::prop_xml_namespace);
}

object document::class_info::create_prototype() {
  local_root_scope scope;

  object proto = node::class_info::create_prototype();

  create_native_method(proto, "dump", 0);
  create_native_method(proto, "copy", 1);
  create_native_method(proto, "toString", 0);

  return proto;
}

string document::dump() {
  xmlChar *doc_txt;
  int doc_txt_len;

  xmlDocDumpFormatMemoryEnc(c_obj(), &doc_txt, &doc_txt_len, "UTF-16", 1);

  char16_t *txt = (char16_t *) doc_txt;
  int txt_len = doc_txt_len / sizeof(char16_t);

  if (txt_len > 0 && *txt == 0xFEFF) {
    ++txt;
    --txt_len;
  }

  string str(txt, txt_len);

  xmlFree(doc_txt);

  return str;
}

object document::copy(bool recursive) {
  xmlDocPtr copy = xmlCopyDoc(c_obj(), recursive);

  if (!copy)
    throw exception("Could not copy XML document");

  return create_native_object<document>(get_prototype(), copy);
}

string document::to_string() {
  return dump();
}

void document::prop_root_element(property_mode mode, value &data) {
  xmlNodePtr node;

  switch (mode) {
  case property_set:
    if (data.is_null() || data.is_void()) {
      node = 0;
      data = object();
    } else if (data.is_object()) {
      node = node::c_from_js(data.get_object());
      if (!node) {
        data = value();
        break;
      }
    } else {
      data = value();
      break;
    }

    if (node && node->type != XML_ELEMENT_NODE) {
      data = object();
      break;
    }

    if (node) {
      xmlDocSetRootElement(c_obj(), node);
    } else {
      xmlNodePtr old = xmlDocGetRootElement(c_obj());
      xmlUnlinkNode(old);
    }
    break;
  case property_get:
    node = xmlDocGetRootElement(c_obj());
    if (!node)
      data = object();
    else
      data = node::create(node);
    break;
  default: break;
  }
}

void document::prop_xml_namespace(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  if (data.is_void()) {
    local_root_scope scope;
    arguments arg;
    arg.push_back(string("xml"));
    data = call("searchNamespaceByPrefix", arg);
  }
}
