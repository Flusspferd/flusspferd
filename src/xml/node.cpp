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
#include <iostream>//FIXME

using namespace flusspferd;
using namespace flusspferd::xml;

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
  std::cout << "DESTROY XML NODE " << ptr << std::endl;
  if (!ptr->parent && ptr->_private == get_gcptr())
    xmlFreeNode(ptr);
}

void node::post_initialize() {
  std::cout << "CREATE XML NODE " << ptr << std::endl;

  register_native_method("copy", &node::copy);
  register_native_method("getDocument", &node::get_document);
}

object node::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "copy", 1);
  create_native_method(proto, "getDocument", 0);

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

object node::copy(bool recursive) {
  xmlNodePtr copy = xmlCopyNode(ptr, recursive);

  if (!copy)
    throw exception("Could not copy XML node");

  return create_native_object<node>(get_prototype(), copy);
}

object node::get_document() {
  if (!ptr->doc)
    return object();

  return create_native_object<document>(object(), ptr->doc);
}
