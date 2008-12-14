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

#include "flusspferd/xml/context.hpp"
#include "flusspferd/xml/document.hpp"
#include "flusspferd/string.hpp"

using namespace flusspferd;
using namespace flusspferd::xml;

xml::context::context(object const &obj, call_context &x)
  : native_object_base(obj)
{
  document &doc = flusspferd::get_native<document>(x.arg[0].to_object());
  define_property("document", doc, read_only_property | permanent_property);

  object ns = create_object();
  ns.set_property("xml", string("http://www.w3.org/XML/1998/namespace"));
  set_property("ns", ns);
}

xml::context::~context() {}

object xml::context::class_info::create_prototype() {
  object proto = create_object();

  return proto;
}
