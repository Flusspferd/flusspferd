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

#ifndef FLUSSPFERD_XML_DOM_IMPLEMENTATION_HPP
#define FLUSSPFERD_XML_DOM_IMPLEMENTATION_HPP

#include "types.hpp"
#include "doctype.hpp"

namespace xml_plugin {

FLUSSPFERD_CLASS_DESCRIPTION(
    dom_implementation,
    (constructible, false)
    (full_name, "xml.DOMImplementation")
    (constructor_name, "DOMImplementation")
    (methods,
      ("hasFeature", bind, hasFeature)
      ("createDocumentType", bind, createDocumentType)
      ("createDocument", bind, createDocument)
    )
) {
public:
  typedef arabica_dom_impl wrapped_type;

  dom_implementation(flusspferd::object const &proto, wrapped_type const &impl);

  bool hasFeature(string_type feature, string_type ver);
  object createDocumentType(string_type qname, string_type pub_id, string_type sys_id);
  object createDocument(string_type ns_uri, string_type qname, doctype &doctype);

  static weak_node_map get_node_map() { return weak_node_map_; }

protected:
  wrapped_type impl_;

  node_map_ptr master_node_map_;

  // Uggh. I dont like using globals, but there isn't much option since Arabica
  // DOMImplementation is a singleton.
  static weak_node_map weak_node_map_;

};

}

#endif
