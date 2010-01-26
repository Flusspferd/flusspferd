// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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

#include <flusspferd/aliases.hpp>

#include "misc_nodes.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;

namespace xml_plugin {

extern void load_parsers(object &exports);
extern void load_exception_class(object &exports);
extern void load_domimpl_class(object &exports);
extern void load_node(object &exports);
extern void load_nodelist(object &exports);
extern void load_doc_classes(object &exports);
extern void load_doctype_class(object &exports);
extern void load_char_classes(object &exports);
extern void load_attr_class(object &exports);
extern void load_element_class(object &exports);
extern void load_namedmap_class(object &exports);

FLUSSPFERD_LOADER_SIMPLE(exports) {

  load_parsers(exports);
  load_exception_class(exports);
  load_domimpl_class(exports);
  load_node(exports);
  load_nodelist(exports);
  load_doc_classes(exports);
  load_doctype_class(exports);
  load_char_classes(exports);
  load_attr_class(exports);
  load_doctype_class(exports);
  load_element_class(exports);
  load_namedmap_class(exports);

  load_class<entity>(exports);
  load_class<entity_ref>(exports);
  load_class<notation>(exports);
  load_class<processing_instruction>(exports);

}

}

