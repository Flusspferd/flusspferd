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

#include "flusspferd/xml/parse.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/string.hpp"
#include <libxml/parser.h>

using namespace flusspferd;
using namespace xml;

object flusspferd::xml::parse_blob(object&, blob &b, object options) {
  value url_v;
  value encoding_v;
  unsigned flags;

  if (options.is_valid()) {
    url_v = options.get_property("url");
    encoding_v = options.get_property("encoding");
    flags = options.get_property("options").to_integral_number(32, false);
  }

  char const *url = url_v.is_string() ? url_v.get_string().c_str() : 0;
  char const *encoding =
    encoding_v.is_string() ? encoding_v.get_string().c_str() : 0;

  flags |= XML_PARSE_NONET;

  xmlDocPtr doc =
    xmlReadMemory((char*) b.get_data(), b.size(), url, encoding, flags);

  if (!doc)
    throw exception("Could not parse XML document");

  return node::create(xmlNodePtr(doc));
}
