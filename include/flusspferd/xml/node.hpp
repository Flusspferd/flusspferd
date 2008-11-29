// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated nodeation files (the "Software"), to deal
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

#ifndef FLUSSPFERD_XML_NODE_HPP
#define FLUSSPFERD_XML_NODE_HPP

#include "../native_object_base.hpp"
#include "../class.hpp"
#include <boost/noncopyable.hpp>
#include <libxml/tree.h>

namespace flusspferd { namespace xml {

class node : public native_object_base {
public:
  node(xmlNodePtr ptr);
  node(call_context &x);
  ~node();

  static object create(xmlNodePtr ptr);
  static void create_all_children(
    xmlNodePtr ptr, bool children = true, bool properties = true);

  xmlNodePtr c_obj() const { return ptr; }

  struct class_info : flusspferd::class_info {
    static char const *constructor_name();
    static std::size_t constructor_arity();

    static object create_prototype();
  };

public:
  static xmlNodePtr c_from_js(object const &v);

protected:
  void set_c_obj(xmlNodePtr ptr) { this->ptr = ptr; }

protected:
  void post_initialize();
  void trace(tracer &);

private: // JS methods
  object copy(bool recursive);
  void unlink();
  void add_content(string const &);
  void add_child(node &child);
  object search_namespace_by_prefix(value const &);
  object search_namespace_by_uri(string const &);
  string to_string();

private: // JS properties
  void prop_name         (property_mode mode, value &data);
  void prop_lang         (property_mode mode, value &data);
  void prop_content      (property_mode mode, value &data);
  void prop_document     (property_mode mode, value &data);
  void prop_parent       (property_mode mode, value &data);
  void prop_next         (property_mode mode, value &data);
  void prop_prev         (property_mode mode, value &data);
  void prop_first_child  (property_mode mode, value &data);
  void prop_last_child   (property_mode mode, value &data);
  void prop_first_sibling(property_mode mode, value &data);
  void prop_last_sibling (property_mode mode, value &data);
  void prop_type         (property_mode mode, value &data);
  void prop_namespace    (property_mode mode, value &data);
  void prop_first_attr   (property_mode mode, value &data);

private:
  xmlNodePtr ptr;
};

}}

#endif
