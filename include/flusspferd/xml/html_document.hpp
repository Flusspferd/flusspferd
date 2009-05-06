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

#ifndef FLUSSPFERD_XML_HTML_DOCUMENT_HPP
#define FLUSSPFERD_XML_HTML_DOCUMENT_HPP

#include "document.hpp"
#include "../class_description.hpp"
#include <libxml/HTMLtree.h>

namespace flusspferd { namespace xml {

FLUSSPFERD_CLASS_DESCRIPTION(
  html_document,
  (base, document)
  (full_name, "XML.HTML.Document")
  (constructor_name, "Document")
  (methods,
    ("dump", bind, dump)))
{
public:
  html_document(object const &, call_context &);
  html_document(object const &, htmlDocPtr doc);
  ~html_document();

public: // JS methods
  string dump();

private:
  void init();
};

}}

#endif
