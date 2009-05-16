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
#include "flusspferd/blob.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/function.hpp"
#include <limits>
#include <cstring>

using namespace flusspferd;

blob::blob(object const &obj, unsigned char const *data, std::size_t length)
  : base_type(obj), data_(data, data+length)
{
  init();
}

blob::blob(object const &obj, call_context &x)
  : base_type(obj)
{
  init();

  local_root_scope scope;
  if (x.arg[0].is_int()) {
    std::size_t length = x.arg[0].get_int();
    data_.resize(length);
    return;
  } else if (x.arg[0].is_object()) {
    object data = x.arg[0].get_object();
    if (data.is_array()) {
      array arr = data;
      std::size_t length = arr.length();
      data_.reserve(length);
      for (std::size_t i = 0; i < length; ++i)
        data_.push_back(el_from_value(arr.get_element(i)));
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
    // TODO: Make this a RangeError once we have error types
    throw exception("Value cannot be a Blob element"); 
  return (unsigned char) x;
}

void blob::init() {
}

blob::~blob() {}

void blob::augment_prototype(object &proto) {
  static const char* js_iterator =
    "function() { return require('Util/Range').Range(0, this.length) }";
  value iter_val = evaluate(js_iterator, strlen(js_iterator));
  proto.define_property("__iterator__", iter_val);

  static const char* js_val_iter =
    "var i = 0;"
    "while (i < this.length) {"
    "  yield this.get(i); i++;"
    "}"
    ;
  function values_fn =
    create_function(
      "values",
      0,
      std::vector<std::string>(),
      string(js_val_iter),
      __FILE__,
      __LINE__);

  proto.define_property("values", value(),
      property_attributes(dont_enumerate, values_fn));

  static const char* js_pairs_iter =
    "function() {"
    "  var i = 0;"
    "  while (i < this.length) {"
    "    yield [i, this.get(i)]; i++;"
    "  }"
    "}";

  function pairs_fn = evaluate(js_pairs_iter, strlen(js_pairs_iter)).get_object();

  proto.define_property("pairs", value(), 
      property_attributes(dont_enumerate, pairs_fn));
}

void blob::set_length(value data) {
  if (!data.is_int())
    throw exception("Blob.length out of range", "RangeError");
  int new_len = data.get_int();
  if (new_len < 0)
    new_len = 0;
  data_.resize(new_len);
}

int blob::get_length() {
  return data_.size();
}

// TODO: Could really do with an enumerate to go with this
bool blob::property_resolve(value const &id, unsigned /*flags*/) {
  if (!id.is_int())
    return false;

  int uid = id.get_int();

  if (uid < 0)
    return false;

  if (size_t(uid) > data_.size())
    return false;
 
  value v = int(data_[uid]);
  define_property(id.to_string(), v, permanent_shared_property);
  return true;
}

void blob::property_op(property_mode mode, value const &id, value &x) {
  int index;
  if (id.is_int()) {
    index = id.get_int();
  } else {
    this->native_object_base::property_op(mode, id, x);
    return;
  }

  if (index < 0 || std::size_t(index) >= data_.size())
    throw exception("Out of bounds of Blob");

  switch (mode) {
  case property_get:
    x = int(data_[index]);
    break;
  case property_set:
    data_[index] = el_from_value(x);
    break;
  default: break;
  };
}

void blob::append(blob const &o) {
  data_.reserve(data_.size() + o.data_.size());
  data_.insert(data_.end(), o.data_.begin(), o.data_.end());
}

object blob::to_array() {
  array arr = create_array(data_.size());
  for (std::size_t i = 0; i < data_.size(); ++i)
    arr.set_element(i, int(data_[i]));
  return arr;
}

object blob::slice(int from, boost::optional<int> to_) {
  int n = data_.size();

  if (from < 0)
    from = n + from;

  int to = to_.get_value_or(n);
  if (to < 0)
    to = n + to;

  if (from > n)
    from = n;
  if (to > n)
    to = n;

  if (to < from)
    to = from;

  return create_native_object<blob>(prototype(), &data_[from], to-from);
}

object blob::clone() {
  return create_native_object<blob>(prototype(), &data_[0], data_.size());
}

string blob::as_utf8() {
  std::size_t strlen = 0;
  for (; strlen < data_.size(); ++strlen)
    if (!data_[strlen])
      break;
  return string((char*) data(), strlen);
}

string blob::as_utf16() {
  return string((char16_t*) data(), size() / sizeof(char16_t));
}

object blob::from_utf8(string const &text) {
  char const *str = text.c_str();
  return create_native_object<blob>(
    object(), (unsigned char const *) str, std::strlen(str));
}

object blob::from_utf16(string const &text) {
  return create_native_object<blob>(
    object(),
    (unsigned char const *) text.data(),
    text.length() * sizeof(char16_t));
}

value blob::set_index(int index, value x) {
  if (index < 0 || std::size_t(index) >= data_.size())
    throw exception("Out of bounds of Blob");

  return data_[index] = el_from_value(x);
}

value blob::get_index(int index) {
  if (index < 0 || std::size_t(index) >= data_.size())
    throw exception("Out of bounds of Blob");

  return data_[index];
}
