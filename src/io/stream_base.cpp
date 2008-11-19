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

#include "flusspferd/io/stream_base.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/string_io.hpp"
#include "flusspferd/create.hpp"
#include <boost/scoped_array.hpp>
#include <cstdlib>

using namespace flusspferd;
using namespace flusspferd::io;

stream_base::stream_base(std::streambuf *p)
  : streambuf(p)
{}

stream_base::~stream_base()
{}

void stream_base::set_streambuf(std::streambuf *p) {
  streambuf = p;
}

void stream_base::post_initialize() {
  register_native_method("readWhole", &stream_base::read_whole);
  register_native_method("read", &stream_base::read);
  register_native_method("write", &stream_base::write);
}

object stream_base::class_info::create_prototype() {
  local_root_scope scope;

  object proto = create_object();

  create_native_method(proto, "readWhole", 0);
  create_native_method(proto, "read", 1);
  create_native_method(proto, "write", 1);

  return proto;
}

string stream_base::read_whole() {
  std::string data;
  char buf[4096];

  std::streamsize length;

  do { 
    length = streambuf->sgetn(buf, sizeof(buf));
    if (length < 0)
      length = 0;
    data.append(buf, length);
  } while (length > 0);

  return string(data);
}

string stream_base::read(unsigned size) {
  if (!size)
    size = 4096;

  boost::scoped_array<char> buf(new char[size + 1]);

  std::streamsize length = streambuf->sgetn(buf.get(), size);
  if (length < 0)
    length = 0;
  buf[length] = '\0';

  return string(buf.get());
}

void stream_base::write(string const &text) {
  char const *str = text.c_str();
  streambuf->sputn(text.c_str(), std::strlen(str));
}

