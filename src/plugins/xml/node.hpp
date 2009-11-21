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


#include <DOM/Node.hpp>

namespace xml_plugin {


#define enum_prop(x) (#x, constant, int(Arabica::DOM::Node_base:: x))
FLUSSPFERD_CLASS_DESCRIPTION(
    node,
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
    (methods,
      ("normalize", bind, normalize)
    )
)
#undef enum_prop
{

public:
  node(flusspferd::object const &proto, flusspferd::call_context &);
  virtual ~node();

  void normalize() { node_.normalize(); }

protected:
  typedef Arabica::DOM::Node<std::string> node_type;

  node(flusspferd::object const &proto);
  node(flusspferd::object const &proto, node_type const &node);

  node_type node_;
};

}

#endif
