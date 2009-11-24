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

#include <flusspferd.hpp>

#include "dom_parser.hpp"
#include "dom_implementation.hpp"
#include "node.hpp"
#include "node_list.hpp"
#include "named_node_map.hpp"
#include "document.hpp"
#include "element.hpp"
#include "char_data.hpp"
#include "attr.hpp"
#include "misc_nodes.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;

namespace xml_plugin {

FLUSSPFERD_LOADER_SIMPLE(exports) {

  load_class<dom_parser>(exports);
  load_class<dom_implementation>(exports);
  load_class<node>(exports);
  load_class<node_list>(exports);
  load_class<named_node_map>(exports);
  load_class<document>(exports);
  load_class<document_fragment>(exports);
  load_class<element>(exports);
  load_class<character_data>(exports);
  load_class<text>(exports);
  load_class<comment>(exports);
  load_class<cdata>(exports);
  load_class<attr>(exports);
  load_class<notation>(exports);
  load_class<entity>(exports);
  load_class<entity_ref>(exports);
  load_class<processing_instruction>(exports);

  // Create the singleton domImplementation
  create<dom_implementation>(
    _container = exports,
    _name = "domImplementation",
    _attributes = read_only_property | permanent_property,
    _arguments = make_vector(
      Arabica::SimpleDOM::DOMImplementation<string_type>::getDOMImplementation()
    )
  );
}

}

