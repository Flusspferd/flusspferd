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

#include "flusspferd/xml/html_document.hpp"
#include "flusspferd/string.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

static htmlDocPtr new_doc(call_context &) {
  htmlDocPtr ptr = htmlNewDoc(0, 0);

  if (!ptr)
    throw exception("Could not create HTML document");

  return ptr;
}

html_document::html_document(object const &obj, call_context &x)
  : base_type(obj, new_doc(x))
{
  init();
}

html_document::html_document(object const &obj, htmlDocPtr ptr)
  : base_type(obj, ptr)
{
  init();
}

html_document::~html_document() {
}

void html_document::init() {
}

string html_document::dump() {
  string result;

  xmlOutputBufferPtr buf = xmlAllocOutputBuffer(0);

  if (!buf)
    throw exception("Could not dump HTML document");

  try {
    htmlDocContentDumpFormatOutput(buf, c_obj(), 0, 1);

    char *data = (char*) buf->buffer->content;
    std::size_t size = buf->buffer->use;

    result = string(data, size);
  } catch (...) {
    xmlOutputBufferClose(buf);
    throw;
  }

  xmlOutputBufferClose(buf);

  return result;
}
