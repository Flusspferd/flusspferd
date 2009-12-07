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
#include "named_node_map.hpp"
#include "dom_exception.hpp"

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;

named_node_map::named_node_map(object const &proto, wrapped_type const &wrap, weak_node_map map)
  : base_type(proto),
    impl_(wrap),
    node_map_(map)
{
}

named_node_map::~named_node_map() {
}

int named_node_map::get_length() {
  return impl_.getLength();
}

object named_node_map::get_node(arabica_node node) {
  node_map_ptr map = node_map_.lock();
  if (!map)
    throw exception("Internal error: node_map has gone away");

  XML_CB_TRY {
    return map->get_node(node);
  } XML_CB_CATCH
}

object named_node_map::item(int idx) {
  arabica_node n = impl_.item(idx);
  XML_CB_TRY {
    return get_node(n);
  } XML_CB_CATCH
}

bool named_node_map::property_resolve(value const &id, unsigned /*flags*/) {
  if (!id.is_int())
    return false;

  int uid = id.get_int();

  if (uid < 0)
    return false;

  if (size_t(uid) >= impl_.getLength())
    return false;
 
  define_property(id, item(uid), permanent_shared_property);
  return true;
}

void named_node_map::property_op(property_mode mode, value const &id, value &x) {
  int index;
  if (id.is_int()) {
    index = id.get_int();
  } else {
    this->native_object_base::property_op(mode, id, x);
    return;
  }

  if (index < 0 || std::size_t(index) >= impl_.getLength())
    throw exception("Out of bounds on NamedNodeMap", "RangeError");

  switch (mode) {
  case property_get:
    x = item(index);
  default: break;
  };
}

object named_node_map::getNamedItem(string_type name) {
  XML_CB_TRY {
    return get_node(impl_.getNamedItem(name));
  } XML_CB_CATCH
}

object named_node_map::setNamedItem(node &arg) {
  XML_CB_TRY {
    return get_node(impl_.setNamedItem(arg.underlying_impl()));
  } XML_CB_CATCH
}

object named_node_map::removeNamedItem(string_type name) {
  XML_CB_TRY {
    return get_node(impl_.removeNamedItem(name));
  } XML_CB_CATCH
}

object named_node_map::getNamedItemNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return get_node(impl_.getNamedItemNS(ns_uri, local_name));
  } XML_CB_CATCH
}

object named_node_map::setNamedItemNS(node &arg) {
  XML_CB_TRY {
    return get_node(impl_.setNamedItemNS(arg.underlying_impl()));
  } XML_CB_CATCH
}

object named_node_map::removeNamedItemNS(string_type ns_uri, string_type local_name) {
  XML_CB_TRY {
    return get_node(impl_.removeNamedItemNS(ns_uri, local_name));
  } XML_CB_CATCH
}

