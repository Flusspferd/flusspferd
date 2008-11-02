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

#include "flusspferd/file_class.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include <fstream>
#include <iostream>

using namespace flusspferd;

class file_class::impl {
public:
  std::fstream stream;
};

file_class::file_class(call_context &x)
  : p(new impl)
{
  if (!x.arg.empty()) {
    string name = x.arg[0].to_string();
    open(name.c_str());
  }
}

file_class::~file_class()
{}

void file_class::post_initialize() {
  register_native_method("open", &file_class::open);
  register_native_method("close", &file_class::close);
  register_native_method("readWhole", &file_class::read_whole);
}

char const *file_class::constructor_name() {
  return "File";
}

std::size_t file_class::constructor_arity() {
  return 0;
}

object file_class::create_prototype() {
  local_root_scope scope;

  object proto = create_object();

  proto.define_property("open", create_native_method("open", 1));
  proto.define_property("close", create_native_method("close", 0));
  proto.define_property("readWhole", create_native_method("readWhole", 0));

  return proto;
}

void file_class::open(char const *name) {
  p->stream.open(name);

  if (!p->stream)
    throw exception("Could not open file");
}

void file_class::close() {
  p->stream.close();
}

string file_class::read_whole() {
  std::string data;
  char buf[4096];

  do { 
    p->stream.read(buf, sizeof(buf));
    data.append(buf, p->stream.gcount());
  } while (p->stream);

  return string(data);
}
