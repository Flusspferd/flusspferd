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
#include <vector>

namespace flusspferd {

/**
 * Binary data storage class.
 *
 * A Blob can store binary data that Javascript cannot otherwise store.
 *
 * Wraps a (mutable) std::vector<unsigned char>.
 *
 * @see <a href="docs/files/src/spidermonkey/blob-jsdoc.html">Javascript 'Blob'</a>.
 */
class blob : public native_object_base {
public:
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
   * Javascript class info.
   */
  struct class_info : flusspferd::class_info {
    static char const *full_name() { return "Blob"; }

    static char const *constructor_name() { return "Blob"; }
    typedef boost::mpl::size_t<1> constructor_arity;

    static object create_prototype();
    static void augment_constructor(object &);
  };

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
  void prop_length(property_mode mode, value &data);

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
