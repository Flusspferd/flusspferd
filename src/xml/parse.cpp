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
#include <libxml/HTMLparser.h>

using namespace flusspferd;
using namespace xml;

namespace {

struct opt {
  char const *url;
  char const *encoding;
  unsigned flags;

  opt(object const &options)
    : url(0), encoding(0), flags(0)
  {
    value url_v;
    value encoding_v;

    if (!options.is_null()) {
      url_v = options.get_property("url");
      encoding_v = options.get_property("encoding");
      flags = options.get_property("options").to_integral_number(32, false);
    }

    if (url_v.is_string())
      url = url_v.get_string().c_str();

    if (encoding_v.is_string())
      encoding = encoding_v.get_string().c_str();
  }
};

}

object flusspferd::xml::parse_blob(object&, blob &b, object options) {
  opt x(options);

  xmlDocPtr doc =
    xmlReadMemory((char*) b.data(), b.size(), x.url, x.encoding, x.flags);

  if (!doc)
    throw exception("Could not parse XML document");

  return node::create(xmlNodePtr(doc));
}

object flusspferd::xml::parse_file(object&, string filename, object options) {
  opt x(options);

  xmlDocPtr doc =
    xmlReadFile(filename.c_str(), x.encoding, x.flags);

  if (!doc)
    throw exception("Could not parse XML document");

  return node::create(xmlNodePtr(doc));
}

object flusspferd::xml::html_parse_blob(object&, blob &b, object options) {
  opt x(options);

  htmlDocPtr doc =
    htmlReadMemory((char*) b.data(), b.size(), x.url, x.encoding, x.flags);

  if (!doc)
    throw exception("Could not parse HTML document");

  return node::create(xmlNodePtr(doc));
}

object flusspferd::xml::html_parse_file(
    object&, string filename, object options)
{
  opt x(options);

  htmlDocPtr doc =
    htmlReadFile(filename.c_str(), x.encoding, x.flags);

  if (!doc)
    throw exception("Could not parse HTML document");

  return node::create(xmlNodePtr(doc));
}
