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

#ifndef FLUSSPFERD_XML_DOCTYPE_HPP
#define FLUSSPFERD_XML_DOCTYPE_HPP

#include "types.hpp"
#include "node.hpp"

namespace xml_plugin {


FLUSSPFERD_CLASS_DESCRIPTION(
    doctype,
    (base, node)
    (constructible, false)
    (full_name, "xml.DocumentType")
    (constructor_name, "DocumentType")
    (properties,
      ("name", getter, getName)
      ("entities", getter, getEntities)
      ("notations", getter, getNotations)
      ("publicId", getter, getPublicId)
      ("systemId", getter, getSystemId)
      ("internalSubset", getter, getInternalSubset)
    )
) {
public:
  typedef arabica_doctype wrapped_type;

  doctype(flusspferd::object const &proto, wrapped_type const &impl, weak_node_map map);

  // Property getters
  string_type getName();
  object getEntities();
  object getNotations();
  string_type getPublicId();
  string_type getSystemId();
  string_type getInternalSubset();

protected:
  wrapped_type impl_;
};

}

#endif

