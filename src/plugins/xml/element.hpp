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

#ifndef FLUSSPFERD_XML_ELEMENT_HPP
#define FLUSSPFERD_XML_ELEMENT_HPP

#include "node.hpp"

#include <DOM/Element.hpp>

namespace xml_plugin {

FLUSSPFERD_CLASS_DESCRIPTION(
    element,
    (base, node)
    (constructible, false)
    (full_name, "xml.Element")
    (constructor_name, "Element")
    (properties,
      ("tagName", getter, getTagName)
    )
    (methods,
      ("getAttribute", bind, getAttribute)
      ("setAttribute", bind, setAttribute)
      ("removeAttribute", bind, removeAttribute)
      ("getAttributeNode", bind, getAttributeNode)
      ("setAttributeNode", bind, setAttributeNode)
      ("removeAttributeNode", bind, removeAttributeNode)
      ("getElementsByTagName", bind, getElementsByTagName)

      ("getAttributeNS", bind, getAttributeNS)
      ("setAttributeNS", bind, setAttributeNS)
      ("removeAttributeNS", bind, removeAttributeNS)
      ("getAttributeNodeNS", bind, getAttributeNodeNS)
      ("setAttributeNodeNS", bind, setAttributeNodeNS)
      // removeAttributeNodeNS is absent from DOM level 2. How odd
      ("getElementsByTagNameNS", bind, getElementsByTagNameNS)

      ("hasAttribute", bind, hasAttribute)
      ("hasAttributeNS", bind, hasAttributeNS)
    )
)
{

public:
  typedef Arabica::DOM::Element<string_type> wrapped_type;

  element(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map);
  virtual ~element();

  // Property getters/setters
  string_type getTagName() { return element_.getTagName(); }

  // Methods
  string_type getAttribute(string_type name);
  void setAttribute(string_type name, string_type value);
  void removeAttribute(string_type name);
  object getAttributeNode(string_type name);
  object setAttributeNode(attr &name);
  object removeAttributeNode(attr &name);
  object getElementsByTagName(string_type name);

  string_type getAttributeNS(string_type ns_uri, string_type local_name);
  void setAttributeNS(string_type ns_uri, string_type local_name, string_type value);
  void removeAttributeNS(string_type ns_uri, string_type local_name);
  object getAttributeNodeNS(string_type ns_uri, string_type local_name);
  object setAttributeNodeNS(attr &a);
  object getElementsByTagNameNS(string_type ns_uri, string_type local_name);

  bool hasAttribute(string_type name);
  bool hasAttributeNS(string_type ns_uri, string_type local_name);

protected:
  wrapped_type element_;
};

}

#endif

