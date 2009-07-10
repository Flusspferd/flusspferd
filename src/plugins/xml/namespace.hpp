// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_XML_NAMESPACE_HPP
#define FLUSSPFERD_XML_NAMESPACE_HPP

#include "flusspferd/class_description.hpp"
#include <boost/noncopyable.hpp>
#include <libxml/tree.h>

namespace flusspferd { namespace xml {

FLUSSPFERD_CLASS_DESCRIPTION(
  namespace_,
  (full_name, "XML.Namespace")
  (constructor_name, "Namespace")
  (constructor_arity, 3)
  (methods,
    ("toString", bind, to_string))
  (properties,
    ("href", getter_setter, (get_href, set_href))
    ("prefix", getter_setter, (get_prefix, set_prefix))))
{
public:
  namespace_(object const &, call_context &);
  namespace_(object const &, xmlNsPtr ptr);
  ~namespace_();

  static object create(xmlNsPtr ptr);

  xmlNsPtr c_obj() const {
    return ptr;
  }

  static xmlNsPtr c_from_js(object const &o);

protected:
  void trace(tracer &);

private:
  void init();

public: // JS methods
  string to_string();

public: // JS properties
  boost::optional<std::string> get_href();
  void set_href(boost::optional<std::string> const &x);
  boost::optional<std::string> get_prefix();
  void set_prefix(boost::optional<std::string> const &x);

private:
  xmlNsPtr ptr;
};

}}

#endif
