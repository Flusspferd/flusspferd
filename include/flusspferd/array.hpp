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

#ifndef FLUSSPFERD_ARRAY_HPP
#define FLUSSPFERD_ARRAY_HPP

#include "object.hpp"

namespace flusspferd {

/**
 * A class for holding a Javascript Array.
 *
 * @see flusspferd::create_array()
 *
 * @ingroup value_types
 * @ingroup property_types
 */
class array : public object {
public:
  /// Construct an #array from a Javascript Array object.
  array(object const &o);

  /// Assign a Javascript Array object to an #array object.
  array &operator=(object const &o);

public:
  /// Get the length of the Array.
  std::size_t length() const;

  /// Set the length of the array, resizing it in the process.
  void set_length(std::size_t);

  /// Get the length of the Array.
  std::size_t size() const { return length(); }
  
  /// Get an array element.
  value get_element(std::size_t n) const;

  /// Set an array element.
  void set_element(std::size_t n, value const &x);

private:
  void check();
};

template<>
struct detail::convert<array> {
  typedef to_value_helper<array> to_value;

  struct from_value {
    root_array root;

    array perform(value const &v) {
      root = array(v.to_object());
      return root;
    }
  };
};

}

#endif
