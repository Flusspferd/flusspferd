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

#include "flusspferd/io/io.hpp"
#include "flusspferd/io/file.hpp"
#include "flusspferd/io/blob_stream.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/class.hpp"
#include "flusspferd/modules.hpp"
#include <iostream>

using namespace flusspferd;
using namespace flusspferd::io;

#ifndef FLUSSPFERD_COVERAGE
FLUSSPFERD_LOADER(container) {
  load_io(container);
}
#endif

object flusspferd::io::load_io(object container) {
  local_root_scope scope;

  object IO = container;

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

  return IO;
}
