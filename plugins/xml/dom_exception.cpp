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

#include "dom_exception.hpp"

using namespace flusspferd;
using namespace xml_plugin;

namespace xml_plugin {
  void load_exception_class(object &exports) {
    load_class<dom_exception>(exports);
  }
}

dom_exception::dom_exception(object const &proto, wrapped_type e)
  : base_type(proto)
{ 
  // Arabica DOM error codes are out by 1
  set_properties("code", int(e.code()+1) )("message", e.what());
}

void dom_exception::augment_prototype(object &proto) {
  // DOMException.prototype.__proto__ = Error.prototype

  // TODO: This can be nicer once #166 is done
  // Get Error.prototype (which isn't the internal [[Prototype]]/prototype() value)
  object const &error_proto = global().get_property_object("Error")
                                      .get_property_object("prototype");

  proto.set_prototype(error_proto);
  proto.set_property("name", "DOMException");
}
