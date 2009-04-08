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

#include "flusspferd/io/io.hpp"
#include "flusspferd/io/file.hpp"
#include "flusspferd/io/blob_stream.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/class.hpp"
#include <iostream>

using namespace flusspferd;
using namespace flusspferd::io;

#ifndef FLUSSPFERD_COVERAGE
extern "C" value flusspferd_load(object container)
{
  return load_io(container);
}
#endif

object flusspferd::io::load_io(object container) {
  local_root_scope scope;

  value previous = container.get_property("IO");

  if (previous.is_object())
    return previous.to_object();

  object IO = current_context().constructor("IO");

  if (IO.is_null()) {
    IO = flusspferd::create_object();

    load_class<stream>(IO);
    load_class<file>(IO);
    load_class<blob_stream>(IO);

    IO.define_property(
      "stdout",
      create_native_object<stream>(object(), std::cout.rdbuf()));

    IO.define_property(
      "stderr",
      create_native_object<stream>(object(), std::cerr.rdbuf()));

    IO.define_property(
      "stdin",
      create_native_object<stream>(object(), std::cin.rdbuf()));

    current_context().add_constructor("IO", IO);
  }

  container.define_property(
    "IO",
    IO,
    object::read_only_property | object::dont_enumerate);

  return IO;
}
