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

#ifndef FLUSSPFERD_XML_HTML_PUSH_PARSER_HPP
#define FLUSSPFERD_XML_HTML_PUSH_PARSER_HPP

#include "../native_object_base.hpp"
#include "../class.hpp"
#include "../blob.hpp"
#include "../class_description.hpp"
#include <libxml/HTMLparser.h>

namespace flusspferd { namespace xml {

FLUSSPFERD_CLASS_DESCRIPTION(
  (cpp_name, html_push_parser)
  (full_name, "XML.HTML.PushParser")
  (constructor_name, "PushParser")
  (constructor_arity, 1)
  (methods,
    ("push", bind, push)
    ("terminate", bind, terminate))
)
{
public:
  html_push_parser(object const &, call_context &);
  ~html_push_parser();

public: // JS methods
  void push(blob &, bool);
  value terminate();

private:
  void terminate2();

  htmlParserCtxtPtr parser;
};

}}

#endif
