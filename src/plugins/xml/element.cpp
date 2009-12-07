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
#include "attr.hpp"

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
  XML_CB_TRY {
    return element_.getAttribute(name);
  } XML_CB_CATCH
}

void element::setAttribute(string_type name, string_type value) {
  XML_CB_TRY {
    return element_.setAttribute(name, value);
  } XML_CB_CATCH
}

void element::removeAttribute(string_type name) {
  XML_CB_TRY {
    element_.removeAttribute(name);
  } XML_CB_CATCH
}

object element::getAttributeNode(string_type name) {
  XML_CB_TRY {
    return get_node(element_.getAttributeNode(name));
  } XML_CB_CATCH
}

object element::setAttributeNode(attr &a) {
  XML_CB_TRY {
    return get_node( element_.setAttributeNode(
      static_cast<arabica_attr>(a.underlying_impl())
    ) );
  } XML_CB_CATCH
}

object element::removeAttributeNode(attr &a) {
  XML_CB_TRY {
    return get_node( element_.removeAttributeNode(
      static_cast<arabica_attr>(a.underlying_impl())
    ) );
  } XML_CB_CATCH
}

object element::getElementsByTagName(string_type tagname) {
  XML_CB_TRY {
    return create<node_list>( make_vector(
      element_.getElementsByTagName(tagname),
      node_map_
    ) );
  } XML_CB_CATCH
}


string_type element::getAttributeNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return element_.getAttributeNS(ns_uri, local_name);
  } XML_CB_CATCH
}

void element::setAttributeNS(string_type ns_uri, string_type local_name, string_type value) {
  XML_CB_TRY {
    element_.setAttributeNS(ns_uri, local_name, value);
  } XML_CB_CATCH
}

void element::removeAttributeNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    element_.removeAttributeNS(ns_uri, local_name);
  } XML_CB_CATCH
}

object element::getAttributeNodeNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return get_node(element_.getAttributeNodeNS(ns_uri, local_name));
  } XML_CB_CATCH
}

object element::setAttributeNodeNS(attr &a) {
  XML_CB_TRY {
    return get_node( element_.setAttributeNodeNS(
      static_cast<arabica_attr>(a.underlying_impl())
    ) );
  } XML_CB_CATCH
}

object element::getElementsByTagNameNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return create<node_list>( make_vector(
      element_.getElementsByTagNameNS(ns_uri, local_name),
      node_map_
    ) );
  } XML_CB_CATCH
}


bool element::hasAttribute(string_type name) {
  XML_CB_TRY {
    return element_.hasAttribute(name);
  } XML_CB_CATCH
}

bool element::hasAttributeNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return element_.hasAttributeNS(ns_uri, local_name);
  } XML_CB_CATCH
}

