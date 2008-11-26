// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated textation files (the "Software"), to deal
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

#include "flusspferd/xml/text.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/tracer.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

text::text(xmlNodePtr ptr)
  : node(ptr)
{}

static xmlNodePtr new_text(call_context &x) {
  value text_v = x.arg[0];
  string text;
  if (!text_v.is_void() && !text_v.is_null())
    text = text_v.to_string();
  xmlChar const *unencoded = (xmlChar const *) text.c_str();
  xmlNodePtr node = xmlNewText(unencoded);
  return node;
}

text::text(call_context &x)
  : node(new_text(x))
{}

text::~text()
{}

void text::post_initialize() {
  node::post_initialize();
}

object text::class_info::create_prototype() {
  local_root_scope scope;

  object proto = node::class_info::create_prototype();

  return proto;
}

char const *text::class_info::constructor_name() {
  return "Text";
}

std::size_t text::class_info::constructor_arity() {
  return 1;
}

