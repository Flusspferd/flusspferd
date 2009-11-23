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

#ifndef FLUSSPFERD_XML_NODE_LIST_HPP
#define FLUSSPFERD_XML_NODE_LIST_HPP

#include "node_map.hpp"

#include <DOM/NodeList.hpp>

namespace xml_plugin {


FLUSSPFERD_CLASS_DESCRIPTION(
    node_list,
    (constructible, false)
    (full_name, "xml.NodeList")
    (constructor_name, "NodeList")
    (properties,
      ("length", getter, get_length)
    )
    (methods,
      ("item", bind, item)
    )
)
{

public:
  typedef Arabica::DOM::NodeList<std::string> wrapped_type;

  node_list(flusspferd::object const &proto, wrapped_type const &node, weak_node_map map);
  virtual ~node_list();

  int get_length();

  flusspferd::object item(int idx);

protected:

  void property_op(property_mode mode, flusspferd::value const &id, flusspferd::value &x);
  bool property_resolve(flusspferd::value const &id, unsigned);

  wrapped_type list_;
  weak_node_map node_map_;
};

}

#endif

