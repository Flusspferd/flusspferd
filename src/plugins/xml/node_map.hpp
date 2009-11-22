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

#ifndef FLUSSPFERD_XML_NODE_MAP_HPP
#define FLUSSPFERD_XML_NODE_MAP_HPP

#include <flusspferd.hpp>
#include <flusspferd/aliases.hpp>

#include "types.hpp"

//#include <iostream>

namespace xml_plugin {

class node_map : public boost::enable_shared_from_this<node_map> {
public:
  /*
   * So, how ever you get hold of a given node in a document, it should always be
   * the same object (in the JS sense):
   *
   *     node === node.firstChild.parentNode
   *
   * We store a map of void* (which is the underlying Node_impl* from Arabica)
   * against the object*. Any function which can return a node should first look
   * in this map, and if found should return the node from there.
   *
   * Each node should store a weak_ptr to the shared_node_map. In the node's
   * destructor it should see if the shared_node_map is alive -- if it is it
   * should remove itself from the map. This is to cope with the fact that
   * finalizers can happen in any order in spidermonkey (i.e. the document could
   * be finalized before the nodes)
   */
  typedef std::map<void*, flusspferd::object> node_identiy_map;
private:
  // Nothing can create instances of us directly
  node_map() {}

protected:
  friend class document;

  // Template this to avoid circular dep on document
  template <class T, class U>
  static node_map_ptr make(T &obj, U node) {
    node_map_ptr map(new node_map());
    void *ptr = static_cast<void*>(node.underlying_impl());
    map->node_map_[ptr] = obj;
    return map;
  }

  node_identiy_map node_map_;

  flusspferd::object create_object_from_node(arabica_node &node);

public:
  ~node_map() {}

  template <class U>
  flusspferd::object get_node(U node) {
    void *ptr = static_cast<void*>(node.underlying_impl());

    //std::cout << "Looking for (" << int(ptr) << ") in the node_map" << std::endl;
    node_identiy_map::const_iterator it = node_map_.find(ptr);

    if (it == node_map_.end()) {
      using namespace flusspferd;
      using namespace flusspferd::aliases;

      object o = create_object_from_node(node);
      //std::cout << "Put something (" << int(ptr) << ") in the node_map" << std::endl;
      node_map_[ptr] = o;
      return o;
    }

    // Should i use dynamic cast here? Probably, but for now, eh.
    //std::cout << "Found (" << int(ptr) << ") in the node_map" << std::endl;
    return it->second;
  }

  template <class U>
  void remove_mapped_node(U node) {
    void *ptr = static_cast<void*>(node.underlying_impl());
    //std::cout << "Removing (" << int(ptr) << ") from the node_map_" << std::endl;
    node_map_.erase(ptr);
  }
};

}

#endif
