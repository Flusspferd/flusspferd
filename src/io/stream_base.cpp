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
#include "flusspferd/blob.hpp"
#include <boost/scoped_array.hpp>
#include <cstdlib>

using namespace flusspferd;
using namespace flusspferd::io;

stream_base::stream_base(object const &o, std::streambuf *p)
  : native_object_base(o), streambuf(p)
{
  register_native_method("readWhole", &stream_base::read_whole);
  register_native_method("read", &stream_base::read);
  register_native_method("readWholeBlob", &stream_base::read_whole_blob);
  register_native_method("readBlob", &stream_base::read_blob);
  register_native_method("write", &stream_base::write);
  register_native_method("print", &stream_base::print);
  register_native_method("flush", &stream_base::flush);

  define_property("fieldSeparator", string(" "));
  define_property("recordSeparator", string("\n"));
  define_property("autoflush", false);
}

stream_base::~stream_base()
{}

void stream_base::set_streambuf(std::streambuf *p) {
  streambuf = p;
}

std::streambuf *stream_base::get_streambuf() {
  return streambuf;
}

object stream_base::class_info::create_prototype() {
  local_root_scope scope;

  object proto = create_object();

  create_native_method(proto, "readWhole", 0);
  create_native_method(proto, "read", 1);
  create_native_method(proto, "readWholeBlob", 0);
  create_native_method(proto, "readBlob", 1);
  create_native_method(proto, "write", 1);
  create_native_method(proto, "print", 0);
  create_native_method(proto, "flush", 0);

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

object stream_base::read_whole_blob() {
  unsigned const N = 4096;

  std::vector<char> data;

  std::streamsize length;

  do {
    data.resize(data.size() + N);
    length = streambuf->sgetn(&data[data.size() - N], N);
    if (length < 0)
      length = 0;
    data.resize(data.size() - N + length);
  } while (length > 0);

  return create_native_object<blob>(
      object(), (unsigned char const *)&data[0], data.size());
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

object stream_base::read_blob(unsigned size) {
  if (!size)
    size = 4096;

  boost::scoped_array<char> buf(new char[size]);

  std::streamsize length = streambuf->sgetn(buf.get(), size);
  if (length < 0)
    length = 0;

  return create_native_object<blob>(
      object(),
      (unsigned char const *) buf.get(),
      length);
}

void stream_base::write(value const &data) {
  if (data.is_string()) {
    string text = data.get_string();
    char const *str = text.c_str();
    streambuf->sputn(text.c_str(), std::strlen(str));
  } else if (data.is_object()) {
    native_object_base *ptr = native_object_base::get_native(data.get_object());
    blob &b = dynamic_cast<blob&>(*ptr);
    streambuf->sputn((char const*) b.get_data(), b.size());
  }
  //TODO slow?
  if (get_property("autoflush").to_boolean())
    flush();
}

void stream_base::print(call_context &x) {
  local_root_scope scope;

  value delim_v = get_property("fieldSeparator");
  string delim;
  if (!delim_v.is_void_or_null())
    delim = delim_v.to_string();

  std::size_t n = x.arg.size();
  for (std::size_t i = 0; i < n; ++i) {
    write(x.arg[i].to_string());
    if (i < n - 1)
      write(delim);
  }

  value record_v = get_property("recordSeparator");
  if (!record_v.is_void_or_null())
    write(record_v.to_string());

  flush();
}

void stream_base::flush() {
  streambuf->pubsync();
}
