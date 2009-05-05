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
#ifndef FLUSSPFERD_BLOB_HPP
#define FLUSSPFERD_BLOB_HPP

#include "native_object_base.hpp"
#include "class.hpp"
#include "class_description.hpp"
#include <vector>

namespace flusspferd {

#ifdef IN_DOXYGEN
/**
 * Binary data storage class.
 *
 * A Blob can store binary data that Javascript cannot otherwise store.
 *
 * Wraps a (mutable) std::vector<unsigned char>.
 *
 * @see <a href="docs/files/src/spidermonkey/blob-jsdoc.html">Javascript 'Blob'</a>.
 *
 * @ingroup jsext
 */
class blob {
#else
FLUSSPFERD_CLASS_DESCRIPTION(
  (cpp_name, blob)
  (full_name, "Blob")
  (constructor_name, "Blob")
  (constructor_arity, 1)
  (augment_prototype, 1)
  (methods,
    ("append", bind, append)
    ("toArray", bind, to_array)
    ("clone", bind, clone)
    ("slice", bind, slice)
    ("asUtf8", bind, as_utf8)
    ("asUtf16", bind, as_utf16)
    ("get", bind, get_index)
    ("set", bind, set_index))
  (properties,
    ("length", getter_setter, (get_length, set_length)))
  (constructor_methods,
    ("fromUtf8", bind_static, from_utf8)
    ("fromUtf16", bind_static, from_utf16))
)
{
#endif
public:
  friend class class_info;

  static void augment_prototype(object &proto);

  /**
   * Javascript constructor.
   *
   * @param obj The object to associate with.
   * @param x The function call_context.
   */
  blob(object const &obj, call_context &x);

  /**
   * Data constructor.
   *
   * Data will be copied.
   *
   * @param obj The object to associate with.
   * @param data Pointer to the data.
   * @param length Length of the data.
   *
   * @see flusspferd::create_native_object
   */
  blob(object const &obj, unsigned char const *data, std::size_t length);

  /// Destructor.
  ~blob();

  /**
   * Access the data.
   *
   * @return Pointer to the data.
   */
  unsigned char *data() { return &data_[0]; }

  /**
   * Get size.
   *
   * @return The size.
   */
  std::size_t size() { return data_.size(); }

  /**
   * Access the data as a std::vector<unsigned char>.
   *
   * @return A reference to the data.
   */
  std::vector<unsigned char> &get() { return data_; }

protected:
  void property_op(property_mode mode, value const &id, value &data);
  bool property_resolve(value const &id, unsigned access);

private: // JS methods
  void append(blob const &o);
  object slice(int from, boost::optional<int> to);
  object to_array();
  object clone();
  string as_utf8();
  string as_utf16();

  value set_index(int index, value x);
  value get_index(int index);

private: // JS properties
  int get_length();
  void set_length(value);

private: // JS constructor methods
  static object from_utf8(string const &);
  static object from_utf16(string const &);

private:
  std::vector<unsigned char> data_;

  static unsigned char el_from_value(value const &);

  void init();
};

}

#endif
