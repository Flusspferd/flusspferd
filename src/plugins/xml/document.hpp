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

#ifndef FLUSSPFERD_XML_DOCUMENT_HPP
#define FLUSSPFERD_XML_DOCUMENT_HPP

#include "node_map.hpp"
#include "node.hpp"

#include <DOM/Document.hpp>

namespace xml_plugin {

FLUSSPFERD_CLASS_DESCRIPTION(
    document,
    (base, node)
    (full_name, "xml.Document")
    (constructor_name, "Document")
    (properties,
      ("doctype", getter, getDoctype)
      ("implementation", getter, getImplementation)
      ("documentElement", getter, getDocumentElement)
    )
    (methods,
      ("createElement", bind, createElement)
      ("createDocumentFragment", bind, createDocumentFragment)
      ("createTextNode", bind, createTextNode)
      ("createComment", bind, createComment)
      ("createCDATASection", bind, createCDATASection)
      ("createProcessingInstruction", bind, createProcessingInstruction)
      ("createAttribute", bind, createAttribute)
      ("createEntityReference", bind, createEntityReference)
      ("getElementsByTagName", bind, getElementsByTagName)
      ("importNode", bind, importNode)
      ("createElementNS", bind, createElementNS)
      ("getElementsByTagNameNS", bind, getElementsByTagNameNS)
      ("getElementById", bind, getElementById)
    )
) {
public:
  typedef arabica_document wrapped_type;

  document(flusspferd::object const &proto, flusspferd::call_context &);

  document(flusspferd::object const &proto, wrapped_type const &doc, weak_node_map map);
  virtual ~document();

  // Property getters
  object getDoctype();
  object getImplementation();
  object getDocumentElement();

  // Methods
  object createElement(string_type tag_name);
  object createDocumentFragment();
  object createTextNode(string_type data);
  object createComment(string_type data);
  object createCDATASection(string_type data);
  object createProcessingInstruction(string_type target, string_type data);
  object createAttribute(string_type name);
  object createEntityReference(string_type name);
  object getElementsByTagName(string_type tag);
  object importNode(node &arg, bool deep);
  object createElementNS(string_type ns_uri, string_type local_name);
  object getElementsByTagNameNS(string_type ns_uri, string_type local_name);
  object getElementById(string_type id);

protected:
  wrapped_type doc_;
};


FLUSSPFERD_CLASS_DESCRIPTION(
    document_fragment,
    (base, node)
    (constructible, false)
    (full_name, "xml.DocumentFragment")
    (constructor_name, "DocumentFragment")
) {
public:
  typedef arabica_doc_fragment wrapped_type;

  document_fragment(object const &proto, wrapped_type const &node, weak_node_map map);
};

} // namespace xml_plugin

#endif
