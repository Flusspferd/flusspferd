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

static char const *DEFAULT_ENCODING = "UTF-8";

using namespace flusspferd;

void flusspferd::load_binary_module(object container) {
  object exports = container.get_property("exports").to_object();
  load_class<binary>(exports);
  load_class<byte_string>(exports);
  load_class<byte_array>(exports);
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

binary::vector_type &binary::get_data() {
  return v_data;
}

std::size_t binary::get_length() const {
  return v_data.size();
}

// -- byte_string -----------------------------------------------------------

byte_string::byte_string(object const &o, call_context &x)
  : base_type(o, boost::ref(x))
{
}

// -- byte_array ------------------------------------------------------------

byte_array::byte_array(object const &o, call_context &x)
  : base_type(o, boost::ref(x))
{
}
