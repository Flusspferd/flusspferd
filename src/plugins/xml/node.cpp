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
#include <flusspferd/aliases.hpp>

#include "node.hpp"
#include "node_list.hpp"

#include <DOM/SAX2DOM/SAX2DOM.hpp>
#include <DOM/io/Stream.hpp>
#include <sstream>

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;

node::node(object const &proto)
  : base_type(proto)
{
}

// Used by document::document
node::node(object const &proto, wrapped_type const &node)
  : base_type(proto),
    node_(node)
{
}

node::node(object const &proto, wrapped_type const &node, weak_node_map map)
  : base_type(proto),
    node_(node),
    node_map_(map)
{
}

node::~node() {
  if (node_map_ptr map = node_map_.lock())
    map->remove_mapped_node(node_);
}

std::string node::to_string() {
  std::ostringstream buf;
  buf << node_;

  return buf.str();
}

value node::getPrefix() {
  return node_.hasPrefix() ?
         value(node_.getPrefix()) :
         value(object());
}


value node::getNamespaceURI() {
  return node_.hasNamespaceURI() ?
         value(node_.getNamespaceURI()) :
         object();
}

object node::get_node(wrapped_type const &n) {
  node_map_ptr map = node_map_.lock();
  if (!map)
    throw exception("Internal error: node_map has gone away");

  return map->get_node<node>(n);
}

object node::getChildNodes() {
  //return create<node_list>( make_vector(node_.getChildNodes()) );
  throw exception("not implemented");
}

object node::getAttributes() {
  throw exception("not implemented");
}

object node::getOwnerDocument() {
  return get_node(node_.getOwnerDocument());
}

