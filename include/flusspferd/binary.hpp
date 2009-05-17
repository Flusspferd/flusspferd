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
#ifndef FLUSSPFERD_BINARY_HPP
#define FLUSSPFERD_BINARY_HPP

#include "native_object_base.hpp"
#include "class_description.hpp"
#include <vector>

namespace flusspferd {

void load_binary_module(object container);

FLUSSPFERD_CLASS_DESCRIPTION(
  binary,
  (full_name, "binary.Binary")
  (constructor_name, "Binary")
  (constructible, 0)
  (methods,
    ("toByteArray", bind, to_byte_array)
    ("toArray", bind, to_array)
    ("indexOf", bind, index_of)
    ("lastIndexOf", bind, last_index_of)
    ("byteAt", bind, byte_at)
    ("slice", bind, slice)
    ("concat", bind, concat)))
{
protected:
  typedef unsigned char element_type;
  typedef std::vector<element_type> vector_type;

  binary(object const &o, call_context &x);
  binary(object const &o, binary const &o);
  binary(object const &o, element_type const *p, std::size_t n);

  virtual binary &create(element_type const *p, std::size_t n) = 0;

  virtual value element(element_type byte) = 0;

protected:
  void property_op(property_mode mode, value const &id, value &data);
  bool property_resolve(value const &id, unsigned access);

protected:
  vector_type &get_data();
  std::size_t set_length(std::size_t);

  void do_append(arguments &x);

public:
  std::size_t get_length();

  vector_type const &get_const_data() { return get_data(); }

public:
  object to_byte_array();
  array to_array();

  int index_of(
    value byte, boost::optional<int> start, boost::optional<int> stop);

  int last_index_of(
    value byte, boost::optional<int> start, boost::optional<int> stop);

  int byte_at(int offset);

  object slice(int begin, boost::optional<int> end);

  void concat(call_context &x);

private:
  std::vector<unsigned char> v_data;
};

FLUSSPFERD_CLASS_DESCRIPTION(
  byte_string,
  (full_name, "binary.ByteString")
  (constructor_name, "ByteString")
  (constructor_arity, 2)
  (base, binary)
  (methods, 
    ("toString", bind, to_string)
    ("toByteString", bind, to_byte_string)
    ("charCodeAt", alias, "byteAt")
    ("charAt", bind, char_at)
    ("substr", bind, substr)
    ("substring", bind, substring))
  (properties,
    ("length", getter, get_length)))
{
public:
  byte_string(object const &o, call_context &x);
  byte_string(object const &o, binary const &o);
  byte_string(object const &o, element_type const *p, std::size_t n);

  virtual binary &create(element_type const *p, std::size_t n);
  virtual value element(element_type byte);

public:
  std::string to_string();
  object to_byte_string();
  object char_at(int offset);
  object substr(int start, boost::optional<int> length);
  object substring(int first, boost::optional<int> last);
};

FLUSSPFERD_CLASS_DESCRIPTION(
  byte_array,
  (full_name, "binary.ByteArray")
  (constructor_name, "ByteArray")
  (constructor_arity, 2)
  (base, binary)
  (methods,
    ("toString", bind, to_string)
    ("toByteString", bind, to_byte_string)
    ("append", bind, append)
    ("push", alias, "append")
    ("pop", bind, pop)
    ("prepend", bind, prepend)
    ("unshift", alias, "prepend")
    ("shift", bind, shift)
    ("reverse", bind, reverse)
    ("sort", bind, sort))
  (properties,
    ("length", getter_setter, (get_length, set_length))))
{
public:
  byte_array(object const &o, call_context &x);
  byte_array(object const &o, binary const &o);
  byte_array(object const &o, element_type const *p, std::size_t n);

  virtual binary &create(element_type const *p, std::size_t n);
  virtual value element(element_type byte);

public:
  std::string to_string();
  object to_byte_string();
  void append(call_context &x);
  int pop();
  void prepend(call_context &x);
  int shift();
  byte_array &reverse();
  binary &sort(object compare);
};

}

#endif
