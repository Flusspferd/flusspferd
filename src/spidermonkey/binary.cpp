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
#include "flusspferd/binary.hpp"
#include <sstream>

static char const *DEFAULT_ENCODING = "UTF-8";

using namespace flusspferd;

void flusspferd::load_binary_module(object container) {
  object exports = container.get_property("exports").to_object();
  load_class<binary>(exports);
  load_class<byte_string>(exports);
  load_class<byte_array>(exports);
}

// -- util ------------------------------------------------------------------

static int get_byte(value byte_) {
  int byte;
  if (byte_.is_int()) {
    byte = byte_.get_int();
    if (byte < 0 || byte > 255)
      throw exception("Byte is outside the valid range for bytes");
    return byte;
  }
  object byte_o = byte_.to_object();
  if (byte_o.is_null())
    throw exception("Not a valid byte");
  binary &byte_bin = flusspferd::get_native<binary>(byte_o);
  if (byte_bin.get_length() != 1)
    throw exception("Byte must not be a non single-element Binary");
  byte = byte_bin.get_const_data()[0];
  return byte;
}

// -- binary ----------------------------------------------------------------

binary::binary(object const &o, call_context &x)
  : base_type(o)
{
  value data = x.arg[0];
  if (data.is_undefined_or_null())
    return;

  if (data.is_number()) {
    if (!data.is_int())
      throw exception("Cannot create binary with non-integer size");
    int i = data.get_int();
    if (i < 0 || i > 2147483647)
      throw exception("Cannot create binary with invalid size");
    v_data.resize(i);
  }

  if (data.is_object()) {
    object o = data.to_object();

    if (o.is_array()) {
      array a(o);
      std::size_t n = a.length();
      vector_type &v = get_data();
      v.resize(n);
      for (std::size_t i = 0; i < n; ++i) {
        value x = a.get_element(i);
        if (!x.is_int())
          throw exception("Can only create Binary from Array of bytes");
        int e = x.get_int();
        if (e < 0 || e > 255)
          throw exception("Can only create Binary from Array of bytes");
        v[i] = e;
      }
    } else {
      try {
        binary &b = flusspferd::get_native<binary>(o);
        v_data = b.v_data;
        return;
      } catch (flusspferd::exception&) {
      }
    }
  }

  string encoding =
    x.arg[1].is_undefined_or_null() ? DEFAULT_ENCODING : string(x.arg[1]);

  string text = data.to_string();

  //TODO
}

binary::binary(object const &o, binary const &b)
  : base_type(o), v_data(b.v_data)
{}

binary::binary(object const &o, element_type const *p, std::size_t n)
  : base_type(o), v_data(p, p + n)
{}

binary::vector_type &binary::get_data() {
  return v_data;
}

std::size_t binary::get_length() {
  return v_data.size();
}

std::size_t binary::set_length(std::size_t n) {
  v_data.resize(n);
  return v_data.size();
}

object binary::to_byte_array() {
  return create_native_object<byte_array>(object(), *this);
}

array binary::to_array() {
  std::size_t n = get_length();
  array a = create_array(n);
  for (std::size_t i = 0; i < n; ++i)
    a.set_element(i, v_data[i]);
  return a;
}

int binary::index_of(
  value byte_, boost::optional<int> start_, boost::optional<int> stop_)
{
  int byte = get_byte(byte_);
  int start = start_.get_value_or(0);
  if (start < 0)
    start = 0;
  int stop = stop_.get_value_or(get_length() - 1);
  if (std::size_t(stop) >= get_length())
    stop = get_length() - 1;

  for (; start <= stop; ++start)
    if (v_data[start] == byte)
      return start;
  return -1;
}

int binary::last_index_of(
  value byte_, boost::optional<int> start_, boost::optional<int> stop_)
{
  int byte = get_byte(byte_);
  int start = start_.get_value_or(0);
  if (start < 0)
    start = 0;
  int stop = stop_.get_value_or(get_length() - 1);
  if (std::size_t(stop) >= get_length())
    stop = get_length() - 1;

  for (; start <= stop; --stop)
    if (v_data[stop] == byte)
      return stop;
  return -1;
}

int binary::byte_at(int offset) {
  if (offset < 0 || std::size_t(offset) > get_length())
    throw exception("Offset outside range", "RangeError");
  return v_data[offset];
}

object binary::slice(int begin, boost::optional<int> end_) {
  int n = get_length();

  if (begin < 0)
    begin = n + begin;

  int end = end_.get_value_or(n);
  if (end < 0)
    end = n + end;

  if (begin > n)
    begin = n;
  if (end > n)
    end = n;

  if (end < begin)
    end = begin;

  return create(&v_data[begin], end - begin);
}

// -- byte_string -----------------------------------------------------------

byte_string::byte_string(object const &o, call_context &x)
  : base_type(o, boost::ref(x))
{}

byte_string::byte_string(object const &o, binary const &b)
  : base_type(o, b)
{}

byte_string::byte_string(object const &o, element_type const *p, std::size_t n)
  : base_type(o, p, n)
{}

object byte_string::create(element_type const *p, std::size_t n) {
  return create_native_object<byte_string>(object(), p, n);
}

std::string byte_string::to_string() {
  std::ostringstream stream;
  stream << "[ByteString " << get_length() << "]";
  return stream.str();
}

object byte_string::to_byte_string() {
  return *this;
}

object byte_string::char_at(int offset) {
  if (offset < 0 || std::size_t(offset) > get_length())
    throw exception("Offset outside range", "RangeError");
  return create(&get_data()[offset], 1);
}

object byte_string::substr(int start, boost::optional<int> length_) {
  if (start < 0)
    start += get_length();
  if (start < 0)
    start = 0;
  if (std::size_t(start) >= get_length())
    start = get_length();
  int length = length_.get_value_or(get_length() - start);
  if (length < 0)
    length = 0;
  if (std::size_t(length) > get_length() - start)
    length = get_length() - start;
  return create(&get_data()[start], length);
}

object byte_string::substring(int first, boost::optional<int> last_) {
  if (first < 0)
    first = 0;
  if (std::size_t(first) > get_length())
    first = get_length();
  int last = last_.get_value_or(get_length());
  if (last < 0)
    last = 0;
  if (std::size_t(last) > get_length())
    last = get_length();
  if (last < first)
    std::swap(first, last);
  return create(&get_data()[first], last - first);
}

// -- byte_array ------------------------------------------------------------

byte_array::byte_array(object const &o, call_context &x)
  : base_type(o, boost::ref(x))
{}

byte_array::byte_array(object const &o, binary const &b)
  : base_type(o, b)
{}

byte_array::byte_array(object const &o, element_type const *p, std::size_t n)
  : base_type(o, p, n)
{}

object byte_array::create(element_type const *p, std::size_t n) {
  return create_native_object<byte_array>(object(), p, n);
}

std::string byte_array::to_string() {
  std::ostringstream stream;
  stream << "[ByteArray " << get_length() << "]";
  return stream.str();
}

object byte_array::to_byte_string() {
  return create_native_object<byte_string>(object(), *this);
}
