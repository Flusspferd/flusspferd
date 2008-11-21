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

#include "flusspferd/xml/document.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/string.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

document::document(xmlDocPtr ptr)
  : ptr(ptr)
{}

document::document(call_context &x) {
  ptr = xmlNewDoc((xmlChar const *) "1.0");
}

document::~document() {
  xmlFreeDoc(ptr);
}

void document::post_initialize() {
  register_native_method("dump", &document::dump);
}

object document::class_info::create_prototype() {
  local_root_scope scope;

  object proto = create_object();

  create_native_method(proto, "dump", 0);

  return proto;
}

char const *document::class_info::constructor_name() {
  return "Document";
}

std::size_t document::class_info::constructor_arity() {
  return 0;
}

string document::dump() {
  xmlChar *doc_txt;
  int doc_txt_len;
  xmlDocDumpFormatMemoryEnc(ptr, &doc_txt, &doc_txt_len, "UTF-16", 1);

  char16_t *txt = (char16_t *) doc_txt;
  int txt_len = doc_txt_len / sizeof(char16_t);

  if (txt_len > 0 && *txt == 0xFEFF) {
    ++txt;
    --txt_len;
  }

  std::basic_string<char16_t> str(txt, txt_len );

  xmlFree(doc_txt);

  return string(str);
}
