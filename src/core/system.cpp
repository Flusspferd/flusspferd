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

#include "flusspferd/system.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/io/stream.hpp"
#include <iostream>
#include <ostream>

using namespace flusspferd;

void flusspferd::load_system_module(object &context) {
  object exports = context.get_property_object("exports");

  context.call("require", "io");

  exports.define_property(
    "stdout",
    create_native_object<io::stream>(object(), std::cout.rdbuf()),
    read_only_property | permanent_property);

  exports.define_property(
    "stderr",
    create_native_object<io::stream>(object(), std::cerr.rdbuf()),
    read_only_property | permanent_property);

  exports.define_property(
    "stdin",
    create_native_object<io::stream>(object(), std::cin.rdbuf()),
    read_only_property | permanent_property);

  try {
    exports.define_property(
      "env",
      context.call("require", "environment"),
      read_only_property | permanent_property);
  } catch (exception &) {
  }

  exports.define_property(
    "args",
    create_array(),
    read_only_property | permanent_property);
}
