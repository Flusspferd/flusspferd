// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_XML_DOCUMENT_HPP
#define FLUSSPFERD_XML_DOCUMENT_HPP

#include "node.hpp"
#include <boost/noncopyable.hpp>
#include <boost/mpl/size_t.hpp>
#include <libxml/tree.h>

namespace flusspferd { namespace xml {

class document : public node {
public:
  struct class_info : node::class_info {
    static char const *full_name() { return "XML.Document"; }

    static char const *constructor_name() { return "Document"; }
    typedef boost::mpl::size_t<0> constructor_arity;

    static object create_prototype();
  };

  document(object const &, call_context &);
  document(object const &, xmlDocPtr doc);
  ~document();

  xmlDocPtr c_obj() const {
    return xmlDocPtr(node::c_obj());
  }

  static xmlDocPtr c_from_js(object const &o);

protected:
  void trace(tracer &);

private:
  void init();

private: // JS methods
  string dump();
  object copy(bool recursive);
  value to_string();

private: // JS properties
  void set_root_element(object const &);
  object get_root_element();
};

}}

#endif
