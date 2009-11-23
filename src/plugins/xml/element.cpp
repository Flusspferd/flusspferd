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

#include "element.hpp"
#include "node_list.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;

element::element(object const &proto, wrapped_type const &node, weak_node_map map)
  : base_type(proto, node, map),
    element_(node)
{ }

element::~element()
{ }


string_type element::getAttribute(string_type name) {
  return element_.getAttribute(name);
}

void element::setAttribute(string_type name, string_type value) {
  return element_.setAttribute(name, value);
}

void element::removeAttribute(string_type name) {
  element_.removeAttribute(name);
}

object element::getAttributeNode(string_type name) {
  throw exception("not implemented");
}

object element::setAttributeNode(string_type name) {
  throw exception("not implemented");
}

object element::removeAttributeNode(string_type name) {
  throw exception("not implemented");
}

object element::getElementsByTagName(string_type tagname) {
  return create<node_list>( make_vector(
    element_.getElementsByTagName(tagname),
    node_map_
  ) );
}


string_type element::getAttributeNS(string_type ns_uri, string_type local_name) {
  return element_.getAttributeNS(ns_uri, local_name);
}

void element::setAttributeNS(string_type ns_uri, string_type local_name, string_type value) {
  element_.setAttributeNS(ns_uri, local_name, value);
}

void element::removeAttributeNS(string_type ns_uri, string_type local_name) {
  element_.removeAttributeNS(ns_uri, local_name);
}

object element::getAttributeNodeNS(string_type ns_uri, string_type local_name) {
  throw exception("not implemented");
}

object element::setAttributeNodeNS(string_type ns_uri, string_type local_name) {
  throw exception("not implemented");
}

object element::removeAttributeNodeNS(string_type ns_uri, string_type local_name) {
  throw exception("not implemented");
}

object element::getElementsByTagNameNS(string_type ns_uri, string_type local_name) {
  return create<node_list>( make_vector(
    element_.getElementsByTagNameNS(ns_uri, local_name),
    node_map_
  ) );
}


bool element::hasAttribute(string_type name) {
  return element_.hasAttribute(name);
}

bool element::hasAttributeNS(string_type ns_uri, string_type local_name) {
  return element_.hasAttributeNS(ns_uri, local_name);
}

