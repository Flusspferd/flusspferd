// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#include "push_parser.hpp"
#include "node.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/tracer.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

push_parser::push_parser(object const &obj, call_context &x)
  : base_type(obj), parser(0)
{
  local_root_scope scope;

  object options = x.arg[0].to_object();

  char const *fname = 0;
  unsigned flags = 0;

  if (!options.is_null()) {
    value fname_v = options.get_property("filename");
    if (!fname_v.is_undefined_or_null())
      fname = fname_v.to_string().c_str();

    flags = options.get_property("options").to_integral_number(32, false);
  }

  parser = xmlCreatePushParserCtxt(0, 0, 0, 0, fname);

  if (!parser)
    throw exception("Could not create parser");

  if (xmlCtxtUseOptions(parser, flags) != 0)
    throw exception("Could not initialise parser options");
}

push_parser::~push_parser() {
  if (parser) {
    xmlFreeDoc(parser->myDoc);
    xmlFreeParserCtxt(parser);
  }
}

void push_parser::push(binary &b, bool t) {
  if (!parser)
    throw exception("Could not parse chunk: parser is empty");

  int status = xmlParseChunk(
    parser,
    reinterpret_cast<char *>(&b.get_data()[0]),
    b.get_length(),
    t);

  if (status != XML_ERR_OK)
    throw exception("Could not parse chunk");

  if (t)
    terminate2();
}

value push_parser::terminate() {
  if (!parser)
    throw exception("Could not terminate empty parser");

  int status = xmlParseChunk(parser, 0, 0, true);

  if (status != XML_ERR_OK)
    throw exception("Could not terminate parser");

  terminate2();

  return get_property("document");
}

void push_parser::terminate2() {
  define_property(
    "document",
    node::create(xmlNodePtr(parser->myDoc)),
    read_only_property | permanent_property);

  parser->myDoc = 0;
  xmlFreeParserCtxt(parser);
  parser = 0;
}
