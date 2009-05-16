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

#include "flusspferd/xml/reference.hpp"
#include "flusspferd/xml/node.hpp"
#include "flusspferd/xml/document.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

reference_::reference_(object const &obj, xmlNodePtr ptr)
  : base_type(obj, ptr)
{}

static xmlNodePtr new_reference(call_context &x) {
  local_root_scope scope;

  object doc_o = x.arg[0].to_object();
  xmlDocPtr doc = document::c_from_js(doc_o);

  value text_v = x.arg[!doc ? 0 : 1];

  if (!text_v.is_string())
    throw exception("Could not create XML entity reference: "
                    "name has to be a string");

  string text = text_v.get_string();

  xmlChar const *data = (xmlChar const *) text.c_str();

  xmlNodePtr result = 0;

  if (text.substr(0, 1) == "#" || text.substr(0, 2) == "&#")
    result = xmlNewCharRef(doc, data);
  else
    result = xmlNewReference(doc, data);

  if (!result)
    throw exception("Could not create XML entity reference");

  return result;
}

reference_::reference_(object const &obj, call_context &x)
  : base_type(obj, new_reference(x))
{}

reference_::~reference_()
{}
