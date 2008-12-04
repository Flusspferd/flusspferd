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
#include <limits>

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
      for (std::size_t i = 0; i < length; ++i) {
        value v = arr.get_element(i);
        if (!v.is_int())
          throw exception("Could not create Blob");
        int x = v.get_int();
        if (x < 0 || x > std::numeric_limits<unsigned char>::max())
          throw exception("Could not create Blob");
        unsigned char ch = x;
        this->data.push_back(ch);
      }
      return;
    }
  }
  throw exception("Could not create Blob");
}

void blob::init() {
  unsigned const RW = permanent_shared_property;

  define_native_property("length", RW, &blob::prop_length);
}

blob::~blob() {}

object blob::class_info::create_prototype() {
  object proto = create_object();
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
