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
#include <boost/mpl/size_t.hpp>
#include <libxml/tree.h>

namespace flusspferd { namespace xml {

class node : public native_object_base {
public:
  node(object const &o, xmlNodePtr ptr);
  node(object const &o, call_context &x);
  ~node();

  static object create(xmlNodePtr ptr);
  static void create_all_children(
    xmlNodePtr ptr, bool children = true, bool properties = true);

  xmlNodePtr c_obj() const { return ptr; }
  void set_c_obj(xmlNodePtr ptr) { this->ptr = ptr; }

  struct class_info : flusspferd::class_info {
    static char const *full_name() { return "XML.Node"; }

    static char const *constructor_name() { return "Node"; }
    typedef boost::mpl::size_t<3> constructor_arity;

    static object create_prototype();
  };

public:
  static xmlNodePtr c_from_js(object const &v);

protected:
  void trace(tracer &);

private:
  void init();

private: // JS methods
  object copy(bool recursive);
  void unlink();
  void purge();
  void add_content(string const &data);
  void add_child(node &child);
  void add_child_list(node &child);
  void add_node(call_context &x);
  void add_namespace(call_context &x);
  void add_attribute(call_context &x);
  void set_attribute(call_context &x);
  void find_attribute(call_context &x);
  void get_attribute(call_context &x);
  void unset_attribute(call_context &x);
  object search_namespace_by_prefix(value const &prefix);
  object search_namespace_by_uri(string const &href);
  string to_string();

private: // JS properties
  std::string get_name();
  void set_name(std::string const &);

  std::string get_type();

  object get_document();

  std::string get_lang();
  void set_lang(std::string const &);

  boost::optional<std::string> get_content();
  void set_content(boost::optional<std::string> const &);

  object get_parent();
  void set_parent(object);

  object get_next_sibling();
  void set_next_sibling(object);

  object get_previous_sibling();
  void set_previous_sibling(object);

  object get_first_child();
  void set_first_child(object);

  object get_last_child();

  object get_first_sibling();

  object get_last_sibling();

  object get_first_attribute();
  void set_first_attribute(object);

  void prop_namespace    (property_mode mode, value &data);
  void prop_namespaces   (property_mode mode, value &data);

private:
  xmlNodePtr ptr;
};

}}

#endif
