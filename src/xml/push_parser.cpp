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

#include "flusspferd/xml/push_parser.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/tracer.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

push_parser::push_parser(object const &obj, call_context &x)
  : native_object_base(obj), parser(0)
{
  if (!x.arg[0].is_void_or_null() && !x.arg[0].is_string())
    throw exception("Could not create parser: filename has to be a string");

  char const *fname = 0;
  if (x.arg[0].is_string())
    fname = x.arg[0].get_string().c_str();

  parser = xmlCreatePushParserCtxt(0, 0, 0, 0, fname);

  if (!parser)
    throw exception("Could not create parser");

  define_native_property(
    "document",
    read_only_property | permanent_shared_property,
    &push_parser::prop_document);
}

push_parser::~push_parser() {
  if (parser)
    xmlFreeParserCtxt(parser);
}

object push_parser::class_info::create_prototype() {
  object proto = create_object();

  return proto;
}

void push_parser::trace(tracer &trc) {
  if (parser->myDoc)
    trc("parser-doc", *static_cast<object*>(parser->myDoc->_private));
}

void push_parser::prop_document(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  data = node::create(xmlNodePtr(parser->myDoc));
}
