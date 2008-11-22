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
#include <iostream>//FIXME

using namespace flusspferd;
using namespace flusspferd::xml;

document::document(xmlDocPtr ptr)
  : ptr(ptr)
{}

document::document(call_context &) {
  ptr = xmlNewDoc((xmlChar const *) "1.0");

  if (!ptr)
    throw exception("Could not create empty XML document");
}

document::~document() {
  std::cout << "DESTROY DOCUMENT " << ptr << std::endl;
  xmlFreeDoc(ptr);
}

void document::post_initialize() {
  std::cout << "CREATE DOCUMENT " << ptr << std::endl;

  ptr->_private = get_gcptr();

  register_native_method("dump", &document::dump);
  register_native_method("copy", &document::copy);
  register_native_method("setRootElement", &document::set_root_element);
  register_native_method("getRootElement", &document::get_root_element);
}

object document::class_info::create_prototype() {
  local_root_scope scope;

  object proto = create_object();

  create_native_method(proto, "dump", 0);
  create_native_method(proto, "copy", 1);
  create_native_method(proto, "setRootElement", 1);
  create_native_method(proto, "getRootElement", 0);

  return proto;
}

char const *document::class_info::constructor_name() {
  return "Document";
}

std::size_t document::class_info::constructor_arity() {
  return 0;
}

void document::trace(tracer &) {
}

string document::dump() {
  xmlChar *doc_txt;
  int doc_txt_len;

  xmlDocDumpFormatMemoryEnc(ptr, &doc_txt, &doc_txt_len, "UTF-16", 1);

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
  xmlDocPtr copy = xmlCopyDoc(ptr, recursive);

  if (!copy)
    throw exception("Could not copy XML document");

  return create_native_object<document>(get_prototype(), copy);
}

void document::set_root_element(node &nd) {
  xmlNodePtr node = nd.c_obj();

  xmlNodePtr old = xmlDocSetRootElement(ptr, node);
  if (old && !old->_private)
    xmlFreeNode(old);

  node->_private = 0;
}

object document::get_root_element() {
  xmlNodePtr root = xmlDocGetRootElement(ptr);

  if (!root)
    return object();

  return create_native_object<node>(object(), root);
}
