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

#ifndef FLUSSPFERD_XML_NODE_HPP
#define FLUSSPFERD_XML_NODE_HPP

#include "node_map.hpp"

#include <DOM/Node.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace xml_plugin {

namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;

#define enum_prop(x) (#x, constant, int(Arabica::DOM::Node_base:: x))
FLUSSPFERD_CLASS_DESCRIPTION(
    node,
    (constructible, false)
    (full_name, "xml.Node")
    (constructor_name, "Node")
    (constructor_properties,
      enum_prop(ELEMENT_NODE)
      enum_prop(ATTRIBUTE_NODE)
      enum_prop(TEXT_NODE)
      enum_prop(CDATA_SECTION_NODE)
      enum_prop(ENTITY_REFERENCE_NODE)
      enum_prop(ENTITY_NODE)
      enum_prop(PROCESSING_INSTRUCTION_NODE)
      enum_prop(COMMENT_NODE)
      enum_prop(DOCUMENT_NODE)
      enum_prop(DOCUMENT_TYPE_NODE)
      enum_prop(DOCUMENT_FRAGMENT_NODE)
      enum_prop(NOTATION_NODE)
      // Do we need max type? Guess it doesn't hurt
      enum_prop(MAX_TYPE)
    )
    (properties,
      ("nodeName", getter, getNodeName)
      ("nodeValue", getter_setter, (getNodeValue, setNodeValue))
      ("nodeType", getter, getNodeType)
      ("parentNode", getter, getParentNode)
      ("childNodes", getter, getChildNodes)
      ("firstChild", getter, getFirstChild)
      ("lastChild", getter, getLastChild)
      ("previousSibling", getter, getPreviousSibling)
      ("nextSibling", getter, getNextSibling)
      ("attributes", getter, getAttributes)
      ("ownerDocument", getter, getOwnerDocument)
      ("namespaceURI", getter, getNamespaceURI)
      ("prefix", getter_setter, (getPrefix, setPrefix))
      ("localName", getter, getLocalName)
    )
    (methods,
      ("toString", bind, to_string)

      ("insertBefore", bind, insertBefore)
      ("replaceChild", bind, replaceChild)
      ("removeChild", bind, removeChild)
      ("appendChild", bind, appendChild)
      ("hasChildNodes", bind, hasChildNodes)
      ("cloneNode", bind, cloneNode)
      ("normalize", bind, normalize)
      ("isSupported", bind, isSupported)
      ("hasAttributes", bind, hasAttributes)
    )
)
#undef enum_prop
{

public:
  typedef arabica_node wrapped_type;

  node(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map);
  virtual ~node();

  arabica_node const & underlying_impl() { return node_; }

  string_type to_string();

  // Property getters/setters
  string_type getNodeName() { return node_.getNodeName(); }
  flusspferd::value getNodeValue();
  void setNodeValue(string_type const &s);
  int getNodeType() { return node_.getNodeType(); }
  object getParentNode() { return get_node(node_.getParentNode()); }
  object getChildNodes();
  object getFirstChild() { return get_node(node_.getFirstChild()); }
  object getLastChild() { return get_node(node_.getLastChild()); }
  object getPreviousSibling() { return get_node(node_.getPreviousSibling()); }
  object getNextSibling() { return get_node(node_.getNextSibling()); }
  object getAttributes();
  object getOwnerDocument();
  flusspferd::value getNamespaceURI();
  flusspferd::value getPrefix();
  void setPrefix(string_type const &s);
  string_type getLocalName() { return node_.getLocalName(); }

  // Methods
  object insertBefore(node &newChild, node &refChild);
  object replaceChild(node &newChild, node &oldChild);
  object removeChild(node &oldChild);
  object appendChild(node &newChild);
  bool hasChildNodes() { return node_.hasChildNodes(); }
  object cloneNode(bool deep);
  void normalize() { node_.normalize(); }
  bool isSupported(string_type feat, string_type ver)
    { return node_.isSupported(feat, ver); }
  bool hasAttributes() { return node_.hasAttributes(); }

protected:

  node(flusspferd::object const &proto);
  // Used by document::document
  node(flusspferd::object const &proto, wrapped_type const &node);

  wrapped_type node_;
  weak_node_map node_map_;

  object get_node(wrapped_type const &node);
};

}

#endif
