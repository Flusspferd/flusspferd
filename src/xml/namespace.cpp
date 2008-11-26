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
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/local_root_scope.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

namespace_::namespace_(xmlNsPtr ptr)
  : ptr(ptr)
{
  if (!ptr->_private)
    ptr->_private = permanent_ptr();
}

namespace_::namespace_(call_context &x) {
  local_root_scope scope;

  value href_v = x.arg[0];
  value prefix_v = x.arg[1];

  xmlChar const *href_p = 0;
  if (!href_v.is_void() && !href_v.is_null())
    href_p = (xmlChar const *) href_v.to_string().c_str();
  xmlChar const *prefix_p = 0;
  if (!prefix_v.is_void() && !prefix_v.is_null())
    prefix_p = (xmlChar const *) prefix_v.to_string().c_str();

  ptr = xmlNewNs(0, href_p, prefix_p);

  ptr->_private = permanent_ptr();
}

namespace_::~namespace_() {
  if (ptr && !ptr->context && ptr->_private == permanent_ptr()) {
    xmlFreeNs(ptr);
  }
}

object namespace_::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "toString", 1);

  return proto;
}

char const *namespace_::class_info::constructor_name() {
  return "Namespace";
}

std::size_t namespace_::class_info::constructor_arity() {
  return 2;
}

void namespace_::post_initialize() {
  if (!ptr->_private)
    ptr->_private = permanent_ptr();

  register_native_method("toString", &namespace_::to_string);
}

void namespace_::trace(tracer &trc) {
  trc("self", from_permanent_ptr(ptr->_private));
  if (ptr->context)
    trc("context", from_permanent_ptr(ptr->context->_private));
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
