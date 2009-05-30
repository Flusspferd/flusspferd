// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
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

#ifndef FLUSSPFERD_XML_CONTEXT_HPP
#define FLUSSPFERD_XML_CONTEXT_HPP

#include "flusspferd/class_description.hpp"
#include <libxml/xpath.h>

namespace flusspferd { namespace xml {

FLUSSPFERD_CLASS_DESCRIPTION(
  xpath_context,
  (full_name, "XML.XPath")
  (constructor_name, "XPath")
  (constructor_arity, 1)
  (properties,
    ("current", getter_setter, (get_current, set_current))))
{
public:
  xpath_context(object const &, call_context &);
  ~xpath_context();

private: // JS methods
  void self_call(call_context &);

public: // JS properties
  object get_current();
  void set_current(object);

private:
  xmlXPathContextPtr xpath_ctx;
};

}}

#endif
