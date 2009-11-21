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

#include "node_list.hpp"
#include "node.hpp"


using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace xml_plugin;

node_list::node_list(object const &proto, call_context &)
  : base_type(proto)
{
}

node_list::node_list(object const &proto, wrapped_type const &list)
  : base_type(proto),
    list_(list)
{
}


node_list::~node_list() {
}

int node_list::get_length() {
  return list_.getLength();
}

bool node_list::property_resolve(value const &id, unsigned /*flags*/) {
  if (!id.is_int())
    return false;

  int uid = id.get_int();

  if (uid < 0)
    return false;

  if (size_t(uid) >= list_.getLength())
    return false;
 
  // TODO: ref-identity for nodes
  //value v = element(v_data[uid]);
  //define_property(id, v, permanent_shared_property);
  //return true;
  return false;
}

void node_list::property_op(property_mode mode, value const &id, value &x) {
  int index;
  if (id.is_int()) {
    index = id.get_int();
  } else {
    this->native_object_base::property_op(mode, id, x);
    return;
  }

  if (index < 0 || std::size_t(index) >= list_.getLength())
    throw exception("Out of bounds on NodeList", "RangeError");

  switch (mode) {
  case property_get:
    // TODO: ref-identity for nodes
    x = create<node>( make_vector(list_.item(index)) );
    break;
  default: break;
  };
}
