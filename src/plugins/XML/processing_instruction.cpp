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

#include "processing_instruction.hpp"
#include "node.hpp"
#include "document.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/exception.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

processing_instruction::processing_instruction(
    object const &obj, xmlNodePtr ptr
  )
  : base_type(obj, ptr)
{}

static xmlNodePtr new_processing_instruction(call_context &x) {
  local_root_scope scope;

  xmlDocPtr doc = document::c_from_js(x.arg[0].to_object());

  std::size_t offset = !doc ? 0 : 1;

  if (!x.arg[offset].is_string() || !x.arg[offset + 1].is_string())
    throw exception("Could not create XML processing instruction: "
                    "name and content have to be strings");

  string name = x.arg[offset].get_string();
  string text = x.arg[offset + 1].get_string();

  xmlChar const *name_ = (xmlChar const *) name.c_str();
  xmlChar const *text_ = (xmlChar const *) text.c_str();

  xmlNodePtr result = xmlNewDocPI(doc, name_, text_);

  if (!result)
    throw exception("Could not create XML processing instruction");

  return result;
}

processing_instruction::processing_instruction(object const &o, call_context &x)
  : base_type(o, new_processing_instruction(x))
{}

processing_instruction::~processing_instruction()
{}
