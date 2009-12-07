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
#include "named_node_map.hpp"
#include "dom_exception.hpp"


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
  XML_CB_TRY {
    std::ostringstream buf;
    buf << node_;

    return buf.str();
  }XML_CB_CATCH
}


void node::setNodeValue(string_type const &s) {
  XML_CB_TRY {
    node_.setNodeValue(s);
  } XML_CB_CATCH
}

void node::setPrefix(string_type const &s) {
  XML_CB_TRY {
    node_.setPrefix(s);
  } XML_CB_CATCH
}

value node::getPrefix() {
  XML_CB_TRY {
    return node_.hasPrefix() ?
           value(node_.getPrefix()) :
           value(object());
  } XML_CB_CATCH
}


value node::getNamespaceURI() {
  XML_CB_TRY {
    return node_.hasNamespaceURI() ?
           value(node_.getNamespaceURI()) :
           object();
  } XML_CB_CATCH
}

object node::get_node(wrapped_type const &n) {
  // n can be an 'empty' node, which we should represent as |null|
  if (!n)
    return object();

  node_map_ptr map = node_map_.lock();
  if (!map)
    throw exception("Internal error: node_map has gone away");

  XML_CB_TRY {
    return map->get_node(n);
  } XML_CB_CATCH
}

object node::getChildNodes() {
  XML_CB_TRY {
    return create<node_list>( make_vector(node_.getChildNodes(), node_map_) );
  } XML_CB_CATCH
}

object node::getAttributes() {
  XML_CB_TRY {
    return create<named_node_map>( make_vector( node_.getAttributes(), node_map_) );
  } XML_CB_CATCH
}

object node::getOwnerDocument() {
  XML_CB_TRY {
    return get_node(node_.getOwnerDocument());
  } XML_CB_CATCH
}

object node::insertBefore(node &newChild, node &refChild) {
  XML_CB_TRY {
    return get_node(node_.insertBefore(newChild.node_, refChild.node_));
  } XML_CB_CATCH
}

object node::replaceChild(node &newChild, node &oldChild) {
  XML_CB_TRY {
    return get_node(node_.insertBefore(newChild.node_, oldChild.node_));
  } XML_CB_CATCH
}

object node::removeChild(node &oldChild) {
  XML_CB_TRY {
    return get_node(node_.removeChild(oldChild.node_));
  } XML_CB_CATCH
}

object node::appendChild(node &newChild) {
  XML_CB_TRY {
    return get_node(node_.appendChild(newChild.node_));
  } XML_CB_CATCH
}

object node::cloneNode(bool deep) {
  XML_CB_TRY {
    return get_node(node_.cloneNode(deep));
  } XML_CB_CATCH
}


