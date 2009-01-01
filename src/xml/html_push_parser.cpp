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

#include "flusspferd/xml/html_push_parser.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/tracer.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

html_push_parser::html_push_parser(object const &obj, call_context &x)
  : native_object_base(obj), parser(0), doc(0)
{
  local_root_scope scope;

  object options = x.arg[0].to_object();

  char const *fname = 0;
  unsigned flags = 0;

  if (options.is_valid()) {
    value fname_v = options.get_property("filename");
    if (!fname_v.is_void_or_null())
      fname = fname_v.to_string().c_str();

    flags = options.get_property("options").to_integral_number(32, false);
  }

  parser = htmlCreatePushParserCtxt(0, 0, 0, 0, fname, XML_CHAR_ENCODING_NONE);

  if (!parser)
    throw exception("Could not create parser");

  if (htmlCtxtUseOptions(parser, flags) != 0)
    throw exception("Could not initialise parser options");

  define_native_property(
    "document",
    read_only_property | permanent_shared_property,
    &html_push_parser::prop_document);

  register_native_method(
    "push",
    &html_push_parser::push);

  register_native_method(
    "terminate",
    &html_push_parser::terminate);
}

html_push_parser::~html_push_parser() {
  if (parser) {
    if (parser->myDoc != doc)
      xmlFreeDoc(parser->myDoc);
    xmlFreeParserCtxt(parser);
  }
}

object html_push_parser::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "push", 2);
  create_native_method(proto, "terminate", 0);

  return proto;
}

void html_push_parser::trace(tracer &trc) {
  if (doc)
    trc("parser-doc", *static_cast<object*>(doc->_private));
}

void html_push_parser::prop_document(property_mode mode, value &data) {
  if (mode != property_get)
    return;

  data = node::create(xmlNodePtr(doc));
}

void html_push_parser::push(blob &b, bool t) {
  if (!parser)
    throw exception("Could not parse chunk: parser is empty");

  int status = htmlParseChunk(parser, (char *) b.get_data(), b.size(), t);

  if (status != XML_ERR_OK)
    throw exception("Could not parse chunk");

  if (t)
    terminate2();
}

object html_push_parser::terminate() {
  int status = htmlParseChunk(parser, 0, 0, true);

  if (status != XML_ERR_OK)
    throw exception("Could not terminate parser");

  terminate2();

  return node::create(xmlNodePtr(doc));
}

void html_push_parser::terminate2() {
  doc = parser->myDoc;
  node::create(xmlNodePtr(doc));

  htmlFreeParserCtxt(parser);
  parser = 0;
}
