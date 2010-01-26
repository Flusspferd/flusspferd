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

#ifndef FLUSSPFERD_XML_MISC_NODES_HPP
#define FLUSSPFERD_XML_MISC_NODES_HPP

#include "node.hpp"

namespace xml_plugin {

FLUSSPFERD_CLASS_DESCRIPTION(
    notation,
    (base, node)
    (constructible, false)
    (full_name, "xml.Notation")
    (constructor_name, "Notation")
    /*(properties,
      ("publicId", getter, getPublicId)
      ("systemId", getter, getSystemId)
    )*/
)
{
public:
  typedef arabica_notation wrapped_type;

  notation(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map)
    : base_type(proto,node, map),
      impl_(node)
  { }

  // Property getters/setters
  //string_type getPublicId() { return impl_.getPublicId(); }
  //string_type getSystemId() { return impl_.getSystemId(); }

  // Methods

protected:
  wrapped_type impl_;
};



FLUSSPFERD_CLASS_DESCRIPTION(
    entity,
    (base, node)
    (constructible, false)
    (full_name, "xml.Entity")
    (constructor_name, "Entity")
    /*
    (properties,
      //("publicId", getter, getPublicId)
      //("systemId", getter, getSystemId)
      //("notationName", getter, getNotationName)
    )
    */
)
{
public:
  typedef arabica_entity wrapped_type;

  entity(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map)
    : base_type(proto,node, map),
      impl_(node)
  { }

  // Property getters/setters
  //string_type getPublicId() { return impl_.getPublicId(); }
  //string_type getSystemId() { return impl_.getSystemId(); }
  //string_type getNotationName() { return impl_.getNotationName(); }

  // Methods

protected:
  wrapped_type impl_;
};



FLUSSPFERD_CLASS_DESCRIPTION(
    entity_ref,
    (base, node)
    (constructible, false)
    (full_name, "xml.EntityReference")
    (constructor_name, "EntityReference")
)
{
public:
  typedef arabica_entity_ref wrapped_type;

  entity_ref(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map)
    : base_type(proto,node, map)
  { }
  // No methods or properties on this class
};



FLUSSPFERD_CLASS_DESCRIPTION(
    processing_instruction,
    (base, node)
    (constructible, false)
    (full_name, "xml.ProcessingInstruction")
    (constructor_name, "ProcessingInstruction")
    (properties,
      ("target", getter, getTarget)
      ("data", getter_setter, (getData, setData))
    )
)
{
public:
  typedef arabica_pi wrapped_type;

  processing_instruction(flusspferd::object const &proto, 
                         wrapped_type const &node, weak_node_map map)
    : base_type(proto,node, map),
      impl_(node)
  { }

  // Property getters/setters
  string_type getTarget() { return impl_.getTarget(); }
  string_type getData() { return impl_.getData(); }
  void setData(string_type s) { impl_.setData(s); }

  // Methods

protected:
  wrapped_type impl_;
};

} // namespace xml_plugin

#endif


