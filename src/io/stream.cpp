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

#include "flusspferd/io/stream.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/string_io.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/binary.hpp"
#include <boost/scoped_array.hpp>
#include <cstdlib>

using namespace flusspferd;
using namespace flusspferd::io;

stream::stream(object const &o, std::streambuf *p)
  : base_type(o), streambuf_(p)
{
}

stream::~stream()
{}

void stream::set_streambuf(std::streambuf *p) {
  streambuf_ = p;
}

std::streambuf *stream::streambuf() {
  return streambuf_;
}

string stream::read_whole() {
  std::string data;
  char buf[4096];

  std::streamsize length;

  do { 
    length = streambuf_->sgetn(buf, sizeof(buf));
    if (length < 0)
      length = 0;
    data.append(buf, length);
  } while (length > 0);

  return string(data);
}

object stream::read_whole_binary(boost::optional<byte_array&> output_) {
  binary &output =
    output_
    ? static_cast<binary&>(output_.get())
    : static_cast<binary&>(
        create_native_object<byte_string>(
          object(),
          static_cast<binary::element_type*>(0),
          0));
  root_object root_obj(output);

  unsigned const N = 4096;

  binary::vector_type &data = output.get_data();

  std::streamsize length;

  do {
    data.resize(data.size() + N);
    length = streambuf_->sgetn(
      reinterpret_cast<char*>(&data[data.size() - N]),
      N);
    if (length < 0)
      length = 0;
    data.resize(data.size() - N + length);
  } while (length > 0);

  return output;
}

string stream::read(unsigned size) {
  if (!size)
    size = 4096;

  boost::scoped_array<char> buf(new char[size + 1]);

  std::streamsize length = streambuf_->sgetn(buf.get(), size);
  if (length < 0)
    length = 0;
  buf[length] = '\0';

  return string(buf.get());
}

object stream::read_binary(unsigned size, boost::optional<byte_array&> output_)
{
  if (!size)
    size = 4096;

  binary &output =
    output_
    ? static_cast<binary&>(output_.get())
    : static_cast<binary&>(
        create_native_object<byte_string>(
          object(),
          static_cast<binary::element_type*>(0),
          0));
  root_object root_obj(output);

  binary::vector_type &data = output.get_data();

  data.resize(data.size() + size);

  std::streamsize length = streambuf_->sgetn(
    reinterpret_cast<char *>(&data[data.size() - size]),
    size);
  if (length < 0)
    length = 0;

  data.resize(data.size() - size + length);

  return output;
}

void stream::write(value const &data) {
  if (data.is_string()) {
    string text = data.get_string();
    char const *str = text.c_str();
    streambuf_->sputn(text.c_str(), std::strlen(str));
  } else if (data.is_object()) {
    binary &b = flusspferd::get_native<binary>(data.get_object());
    streambuf_->sputn((char const*) &b.get_data()[0], b.get_length());
  } else {
    throw exception("Cannot write non-object non-string value to Stream");
  }
  //TODO slow?
  if (get_property("autoFlush").to_boolean())
    flush();
}

void stream::flush() {
  streambuf_->pubsync();
}

void stream::print(call_context &x) {
  local_root_scope scope;

  value delim_v = get_property("fieldSeparator");
  string delim;
  if (!delim_v.is_undefined_or_null())
    delim = delim_v.to_string();

  std::size_t n = x.arg.size();
  for (std::size_t i = 0; i < n; ++i) {
    value p = x.arg[i];

    if (p.is_object() && p.get_object().is_array()) {
      value recordSep = get_property("recordSeparator");

      array arr = p.get_object();
      arguments arg;
      std::size_t length = arr.length();
      for (std::size_t i = 0; i < length; ++i)
        arg.push_back(arr.get_element(i));

      set_property("recordSeparator", value());
      call("print", arg);
      set_property("recordSeparator", recordSep);
    } else {
      write(p.to_string());
    }

    if (i < n - 1)
      write(delim);
  }

  value record_v = get_property("recordSeparator");
  if (!record_v.is_undefined_or_null())
    write(record_v.to_string());

  flush();
}

string stream::read_line(value sep_) {
  local_root_scope scope;

  if (sep_.is_undefined_or_null())
    sep_ = string("\n");

  string sep = sep_.to_string();

  if (sep.length() != 1)
    throw exception("Line separators with size other than 1 are not supported");

  int sepc = sep.data()[0];

  if (sepc >= 128)
    throw exception("Non-ASCII line separators are not supported");

  std::string line;

  for (;;) {
    int ch = streambuf_->sbumpc();
    if (ch == std::char_traits<char>::eof())
      break;
    line += ch;
    if (ch == sepc)
      break;
  }

  return line;
}

