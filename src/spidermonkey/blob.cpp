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
#include "flusspferd/blob.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/string.hpp"
#include <limits>
#include <cstring>

using namespace flusspferd;

blob::blob(object const &obj, unsigned char const *data, std::size_t length)
  : native_object_base(obj), data(data, data+length)
{
  init();
}

blob::blob(object const &obj, call_context &x)
  : native_object_base(obj)
{
  init();

  local_root_scope scope;
  if (x.arg[0].is_int()) {
    std::size_t length = x.arg[0].get_int();
    data.resize(length);
    return;
  } else if (x.arg[0].is_object()) {
    object data = x.arg[0].get_object();
    if (data.is_array()) {
      array arr = data;
      std::size_t length = arr.get_length();
      this->data.reserve(length);
      for (std::size_t i = 0; i < length; ++i)
        this->data.push_back(el_from_value(arr.get_element(i)));
      return;
    }
  }
  throw exception("Could not create Blob");
}

unsigned char blob::el_from_value(value const &v) {
  if (!v.is_int())
    throw exception("Value cannot be a Blob element");
  int x = v.get_int();
  if (x < 0 || x > std::numeric_limits<unsigned char>::max())
    // TODO: Make this a RangeError once we have error type
    throw exception("Value cannot be a Blob element"); 
  return (unsigned char) x;
}

void blob::class_info::augment_constructor(object &ctor) {
  create_native_function(ctor, "fromUtf8", &blob::from_utf8);
}

void blob::init() {
  unsigned const RW = permanent_shared_property;

  define_native_property("length", RW, &blob::prop_length);

  register_native_method("append", &blob::append);
  register_native_method("toArray", &blob::to_array);
  register_native_method("copy", &blob::copy);
  register_native_method("asUtf8", &blob::as_utf8);
}

blob::~blob() {}

object blob::class_info::create_prototype() {
  object proto = create_object();

  create_native_method(proto, "append", 1);
  create_native_method(proto, "toArray", 0);
  create_native_method(proto, "copy", 0);
  create_native_method(proto, "asUtf8", 0);

  return proto;
}

void blob::prop_length(property_mode mode, value &data) {
  switch (mode) {
  case property_set:
  {
    int new_len = data.get_int();
    if (new_len < 0)
      new_len = 0;
    this->data.resize(new_len);
    data = new_len;
    break;
  }
  case property_get:
    data = (int)this->data.size();
    break;
  default: break;
  }
}

// TODO: Could really do with an enumerate to go with this
bool blob::property_resolve(value const &id, unsigned ) {
  if (!id.is_number())
    return false;

  // Dont support anything larger than 2**30 in a blob. 1gig limit seems sane
  int uid = id.to_integral_number(30, false);

  if ((size_t)uid > data.size())
    return false;
 
  value v = int(data[uid]);
  define_property(string(uid), v, permanent_shared_property);
  return true;
}

void blob::property_op(property_mode mode, value const &id, value &x) {
  if (id.is_int()) {
    int index = id.get_int();
    if (index < 0 || std::size_t(index) >= data.size())
      throw exception("Out of bounds of Blob");
    switch (mode) {
    case property_get:
      x = int(data[index]);
      break;
    case property_set:
      data[index] = el_from_value(x);
      break;
    default: break;
    };
    return;
  }
  this->native_object_base::property_op(mode, id, x);
}

void blob::append(blob const &o) {
  data.reserve(data.size() + o.data.size());
  data.insert(data.end(), o.data.begin(), o.data.end());
}

object blob::to_array() {
  array arr = create_array(data.size());
  for (std::size_t i = 0; i < data.size(); ++i)
    arr.set_element(i, int(data[i]));
  return arr;
}

object blob::copy() {
  return create_native_object<blob>(get_prototype(), &data[0], data.size());
}

string blob::as_utf8() {
  std::size_t strlen = 0;
  for (; strlen < data.size(); ++strlen)
    if (!data[strlen])
      break;
  return string((char*) get_data(), strlen);
}

object blob::from_utf8(string const &text) {
  char const *str = text.c_str();
  return create_native_object<blob>(
    object(), (unsigned char const *) str, std::strlen(str));
}
