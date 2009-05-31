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
#include "flusspferd/evaluate.hpp"
#include <sstream>
#include <algorithm>

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

void binary::augment_prototype(object &proto) {
  static const char* js_iterator =
    "function() { return require('Util/Range').Range(0, this.length) }";
  value iter_val = evaluate(js_iterator, strlen(js_iterator));
  proto.define_property("__iterator__", iter_val);

  static const char* js_val_iter =
    "var i = 0;"
    "while (i < this.length) {"
    "  yield this[i];"
    "  ++i;"
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
    "    yield [i, this[i]];"
    "    ++i;"
    "  }"
    "}";

  function pairs_fn =
      evaluate(js_pairs_iter, strlen(js_pairs_iter)).get_object();

  proto.define_property("pairs", value(),
      property_attributes(dont_enumerate, pairs_fn));
}

bool binary::property_resolve(value const &id, unsigned /*flags*/) {
  if (!id.is_int())
    return false;

  int uid = id.get_int();

  if (uid < 0)
    return false;

  if (size_t(uid) > v_data.size())
    return false;
 
  value v = element(v_data[uid]);
  define_property(id.to_string(), v, permanent_shared_property);
  return true;
}

void binary::property_op(property_mode mode, value const &id, value &x) {
  int index;
  if (id.is_int()) {
    index = id.get_int();
  } else {
    this->native_object_base::property_op(mode, id, x);
    return;
  }

  if (index < 0 || std::size_t(index) >= v_data.size())
    throw exception("Out of bounds of Blob");

  switch (mode) {
  case property_get:
    x = element(v_data[index]);
    break;
  case property_set:
    v_data[index] = get_byte(x);
    break;
  default: break;
  };
}

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

byte_string &binary::byte_at(int offset) {
  if (offset < 0 || std::size_t(offset) > get_length())
    throw exception("Offset outside range", "RangeError");
  return create_native_object<byte_string>(object(), &v_data[offset], 1);
}

int binary::get(int offset) {
  if (offset < 0 || std::size_t(offset) > get_length())
    throw exception("Offset outside range", "RangeError");
  return v_data[offset];
}

std::pair<std::size_t, std::size_t>
binary::range(int begin, boost::optional<int> end_) {
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

  return std::pair<std::size_t, std::size_t>(begin, end);
}

std::pair<std::size_t, std::size_t>
binary::length_range(int start, boost::optional<int> length_) {
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
  return std::pair<std::size_t, std::size_t>(start, start + length);
}

object binary::slice(int begin, boost::optional<int> end) {
  std::pair<std::size_t, std::size_t> x = range(begin, end);
  return create(&v_data[x.first], x.second - x.first);
}

void binary::concat(call_context &x) {
  local_root_scope scope;
  binary &res = create(&v_data[0], v_data.size()); //copy
  res.do_append(x.arg);
  x.result = res;
}

void binary::do_append(arguments &arg) {
  vector_type &out = v_data;
  for (arguments::iterator it = arg.begin(); it != arg.end(); ++it) {
    value el = *it;
    if (el.is_int()) {
      int x = el.get_int();
      if (x < 0 || x > 255)
        throw exception("Outside byte range", "Range error");
      out.push_back(element_type(x));
    } else if (el.is_object()) {
      object o = el.get_object();
      if (o.is_array()) {
        array a(o);
        std::size_t n = a.length();
        out.reserve(out.size() + n);
        for (std::size_t i = 0; i < n; ++i) {
          value v = a.get_element(i);
          if (!v.is_int())
            throw exception("Must be Array of Numbers");
          int x = v.get_int();
          if (x < 0 || x > 255)
            throw exception("Outside byte range", "RangeError");
          out.push_back(element_type(x));
        }
      } else {
        binary &x = flusspferd::get_native<binary>(o);
        out.insert(out.end(), x.v_data.begin(), x.v_data.end());
      }
    }
  }
}

array binary::split(value delim, object options) {
  local_root_scope scope;
  std::vector<binary*> delims;

  // Parse the delimiter into the vector of delims.
  if (delim.is_int()) { // single Number
    if (delim.get_int() < 0 || delim.get_int() > 255)
      throw exception("Outside byte range", "RangeError");
    element_type e = delim.get_int();
    delims.push_back(&create(&e, 1));
  } else if (delim.is_object()) {
    object obj = delim.get_object();

    if (obj.is_array()) { // Array
      array arr(obj);
      std::size_t n = arr.length();

      for (std::size_t i = 0; i < n; ++i) {
        binary &new_delim =
          create_native_object<byte_string>(object(), (element_type*)0, 0);
        arguments arg;
        arg.push_back(arr.get_element(i));
        new_delim.do_append(arg);
        if (new_delim.get_length() > 0)
          delims.push_back(&new_delim);
      }
    } else { // Binary
      binary &b = flusspferd::get_native<binary>(obj);
      if (b.get_length() > 0)
        delims.push_back(&b);
    }
  }

  if (delims.empty())
    throw exception("Need at least one valid delimiter");

  // Options
  std::size_t count = std::numeric_limits<std::size_t>::max();
  bool include_delimiter = false;

  // (only search options if the options object is not null)
  if (!options.is_null()) {
    // "count" option
    value count_ = options.get_property("count");
    if (!count_.is_undefined_or_null())
      count = count_.to_integral_number(32, 0);

    // "includeDelimiter" option
    value include_delimiter_ = options.get_property("includeDelimiter");
    if (!include_delimiter_.is_undefined_or_null())
      include_delimiter = include_delimiter_.to_boolean();
  }

  // Main loop

  typedef vector_type::iterator iterator;
  iterator pos = v_data.begin();

  array results = create_array();

  // Loop only through the first count-1 elements
  for (std::size_t n = 1; n < count; ++n) {
    // Search for the first occurring delimiter
    std::size_t delim_id = delims.size();
    iterator first_found = v_data.end();
    for (std::size_t i = 0; i < delims.size(); ++i) {
      binary &delim = *delims[i];
      iterator found = std::search(
        pos, v_data.end(),
        delim.get_data().begin(), delim.get_data().end());
      if (found < first_found) {
        first_found = found;
        delim_id = i;
      }
    }

    // No delimiter found
    if (delim_id == delims.size())
      break;

    binary &elem = create_range(pos, first_found);

    // Add element
    results.call("push", elem);

    // Possible add delimiter
    if (include_delimiter)
      results.call("push", *delims[delim_id]);

    // Advance position _after_ the delimiter.
    pos = first_found + delims[delim_id]->get_length();
  }

  // Add last element, possibly containing delimiters
  results.call("push", create_range(pos, v_data.end()));

  return results;
}

void binary::debug_rep(std::ostream &stream) {
  stream << "length:" << v_data.size();
  std::size_t n = std::min(v_data.size(), std::size_t(10));
  if (n)
    stream << " -- ";
  for (std::size_t i = 0; i < n; ++i) {
    if (i)
      stream << ',';
    stream << int(v_data[i]);
  }
  if (n < v_data.size())
    stream << "...";
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

binary &byte_string::create(element_type const *p, std::size_t n) {
  return create_native_object<byte_string>(object(), p, n);
}

value byte_string::element(element_type e) {
  return create(&e, 1);
}

std::string byte_string::to_string() {
  std::ostringstream stream;
  stream << "[ByteString ";
  debug_rep(stream);
  stream << "]";
  return stream.str();
}

object byte_string::to_byte_string() {
  return *this;
}

object byte_string::substr(int start, boost::optional<int> length) {
  std::pair<std::size_t, std::size_t> x = length_range(start, length);
  return create(&get_data()[x.first], x.second - x.first);
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

std::string byte_string::to_source() {
  std::ostringstream out;
  out << "(ByteString([";
  for (vector_type::iterator it = get_data().begin();
      it != get_data().end();
      ++it)
  {
    if (it != get_data().begin())
      out << ",";
    out << int(*it);
  }
  out << "]))";
  return out.str();
}

byte_string &byte_string::join(array &arr, binary &delimiter)
{
  byte_string &res =
    create_native_object<byte_string>(object(), (element_type*) 0, 0);
  root_object root_obj(res);
  std::size_t n = arr.length();
  for (std::size_t i = 0; i < n; ++i) {
    arguments arg;
    if (i > 0)
      arg.push_back(delimiter);
    arg.push_back(arr.get_element(i));
    res.do_append(arg);
  }
  return res;
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

binary &byte_array::create(element_type const *p, std::size_t n) {
  return create_native_object<byte_array>(object(), p, n);
}

value byte_array::element(element_type e) {
  return value(int(e));
}

std::string byte_array::to_string() {
  std::ostringstream stream;
  stream << "[ByteArray ";
  debug_rep(stream);
  stream << "]";
  return stream.str();
}

object byte_array::to_byte_string() {
  return create_native_object<byte_string>(object(), *this);
}

void byte_array::append(call_context &x) {
  do_append(x.arg);
  x.result = int(get_length());
}

int byte_array::pop() {
  if (get_data().empty())
    throw exception("Cannot pop() from empty ByteArray");
  int result = get_data().back();
  get_data().pop_back();
  return result;
}

void byte_array::prepend(call_context &x) {
  vector_type tmp;
  tmp.swap(get_data());
  do_append(x.arg);
  get_data().insert(get_data().end(), tmp.begin(), tmp.end());
  x.result = int(get_length());
}

int byte_array::shift() {
  if (get_data().empty())
    throw exception("Cannot pop() from empty ByteArray");
  int result = get_data().front();
  get_data().erase(get_data().begin());
  return result;
}

byte_array &byte_array::reverse() {
  std::reverse(get_data().begin(), get_data().end());
  return *this;
}

namespace {
  struct compare_helper {
    object &compare;

    bool operator()(unsigned char a, unsigned char b) {
      double val = compare.call(flusspferd::scope_chain(), a, b).to_number();
      return val < 0;
    }
  };
}

binary &byte_array::sort(object compare) {
  if (compare.is_null()) {
    std::sort(get_data().begin(), get_data().end());
  } else {
    compare_helper h = { compare };
    std::sort(get_data().begin(), get_data().end(), h);
  }
  return *this;
}

int byte_array::erase(int begin, boost::optional<int> end) {
  std::pair<std::size_t, std::size_t> x = range(begin, end);
  get_data().erase(get_data().begin() + x.first, get_data().begin() + x.second);
  return get_length();
}

void byte_array::displace(call_context &x) {
  int begin = x.arg[0].to_integral_number(32, true);
  boost::optional<int> end;
  if (!x.arg[1].is_undefined_or_null())
    end = x.arg[1].to_integral_number(32, true);
  std::pair<std::size_t, std::size_t> r = range(begin, end);
  vector_type tmp(get_data().begin() + r.second, get_data().end());
  get_data().erase(get_data().begin() + r.first, get_data().end());
  arguments arg;
  for (std::size_t i = 2; i < x.arg.size(); ++i)
    arg.push_back(x.arg[i]);
  do_append(arg);
  get_data().insert(get_data().end(), tmp.begin(), tmp.end());
  x.result = int(get_length());
}

void byte_array::insert(call_context &x) {
  if (x.arg.empty()) {
    x.result = int(get_length());
    return;
  }
  arguments arg;
  arguments::iterator it = x.arg.begin();
  arg.push_back(*it);
  arg.push_back(*it);
  while (++it != x.arg.end())
    arg.push_back(*it);
  x.arg = arg;
  displace(x);
}

void byte_array::splice(call_context &x) {
  int begin = x.arg[0].to_integral_number(32, true);
  boost::optional<int> length;
  if (!x.arg[1].is_undefined_or_null())
    length = x.arg[1].to_integral_number(32, true);
  std::pair<std::size_t, std::size_t> r = length_range(begin, length);
  root_object o(create(&get_data()[r.first], r.second - r.first));
  x.arg[0] = int(r.first);
  x.arg[1] = int(r.second);
  displace(x);
  x.result = o;
}

byte_array &byte_array::filter(function callback, object thisObj) {
  if (thisObj.is_null())
    thisObj = flusspferd::scope_chain();

  byte_array &result =
    create_native_object<byte_array>(object(), (element_type*)0, 0);
  root_object root_obj(result);

  vector_type &v = get_data();

  for (std::size_t i = 0; i < v.size(); ++i) {
    if (callback.call(thisObj, v[i], i, *this).to_boolean())
      result.get_data().push_back(v[i]);
  }

  return result;
}

void byte_array::for_each(function callback, object thisObj) {
  if (thisObj.is_null())
    thisObj = flusspferd::scope_chain();

  vector_type &v = get_data();

  for (std::size_t i = 0; i < v.size(); ++i)
    callback.call(thisObj, v[i], i, *this);
}

bool byte_array::every(function callback, object thisObj) {
  if (thisObj.is_null())
    thisObj = flusspferd::scope_chain();

  vector_type &v = get_data();

  for (std::size_t i = 0; i < v.size(); ++i)
    if (!callback.call(thisObj, v[i], i, *this).to_boolean())
      return false;

  return true;
}

bool byte_array::some(function callback, object thisObj) {
  if (thisObj.is_null())
    thisObj = flusspferd::scope_chain();

  vector_type &v = get_data();

  for (std::size_t i = 0; i < v.size(); ++i)
    if (callback.call(thisObj, v[i], i, *this).to_boolean())
      return true;

  return false;
}

int byte_array::count(function callback, object thisObj) {
  if (thisObj.is_null())
    thisObj = flusspferd::scope_chain();

  vector_type &v = get_data();

  int n = 0;

  for (std::size_t i = 0; i < v.size(); ++i)
    if (callback.call(thisObj, v[i], i, *this).to_boolean())
      ++n;

  return n;
}

byte_array &byte_array::map(function callback, object thisObj) {
  if (thisObj.is_null())
    thisObj = flusspferd::scope_chain();

  byte_array &result =
    create_native_object<byte_array>(object(), (element_type*)0, 0);
  root_object root_obj(result);

  vector_type &v = get_data();

  result.get_data().reserve(v.size());

  root_value x;

  for (std::size_t i = 0; i < v.size(); ++i) {
    x = callback.call(thisObj, v[i], i, *this);
    arguments arg;
    arg.push_back(x);
    result.do_append(arg);
  }

  return result;
}

value byte_array::reduce(function callback, value initial_value) {
  root_value result(initial_value);

  vector_type &v = get_data();
  object obj = flusspferd::scope_chain();

  for (std::size_t i = 0; i < v.size(); ++i)
    result = callback.call(obj, result, v[i], i, *this);

  return result;
}

value byte_array::reduce_right(function callback, value initial_value) {
  root_value result(initial_value);

  vector_type &v = get_data();
  object obj = flusspferd::scope_chain();

  std::size_t i = v.size();

  while (i--)
    result = callback.call(obj, result, v[i], i, *this);

  return result;
}

std::string byte_array::to_source() {
  std::ostringstream out;
  out << "(ByteArray([";
  for (vector_type::iterator it = get_data().begin();
      it != get_data().end();
      ++it)
  {
    if (it != get_data().begin())
      out << ",";
    out << int(*it);
  }
  out << "]))";
  return out.str();
}
