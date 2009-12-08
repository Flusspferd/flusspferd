// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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

#ifndef FLUSSPFERD_STRING_HPP
#define FLUSSPFERD_STRING_HPP

#include "convert.hpp"
#include "spidermonkey/string.hpp"
#include <string>

namespace flusspferd {

class value;

/**
 * A Javascript string.
 *
 * @ingroup value_types
 */
class string : public Impl::string_impl {
public:
  /// Construct an empty string.
  string();

  /**
   * Construct a string from a value.
   *
   * @param v The value to convert to a string.
   */
  string(value const &v);

  /**
   * Construct a string from a UTF-8 input string.
   *
   * @param str The UTF-8 string.
   * @param length The length in bytes. If this is <code>0</code>, std::strlen
   *            will be used to determine the length.
   */
  string(char const *str, std::size_t length = 0);

  /**
   * Construct a string from a UTF-16 input string.
   *
   * @param str The UTF-16 string.
   * @param length The length in UTF-16 words.
   */
  string(js_char16_t const *str, std::size_t length);

  /**
   * Construct a string from a UTF-8 std::string.
   *
   * @param s The std::string.
   */
  string(std::string const &s);

  /**
   * Construct a string from a UTF-16 std::basic_string.
   *
   * @param s The std::basic_string.
   */
  string(std::basic_string<js_char16_t> const &s);

#ifndef IN_DOXYGEN
  string(Impl::string_impl const &s)
    : Impl::string_impl(s)
  { }
#endif

  /// Destructor.
  ~string();

  /**
   * Assignment operator.
   *
   * @param o The string to assign.
   */
  string &operator=(string const &o);

  /**
   * Get the length of the string.
   *
   * Measured in UTF-16 words.
   *
   * @return The length.
   */
  std::size_t length() const;

  /**
   * Get the length of the string.
   *
   * Measured in UTF-16 words.
   *
   * @return The length.
   */
  std::size_t size() const { return length(); }

  /**
   * Check if the string is empty.
   *
   * @return Whether the string is empty.
   */
  bool empty() const {
    return !length();
  }

  /**
   * Convert to std::string.
   *
   * @return The std::string.
   */
  std::string to_string() const;

  /**
   * Get C string.
   *
   * Valid only while the flusspferd::string is valid.
   *
   * @return The C string.
   */
  char const *c_str() const;

  /**
   * Convert to a UTF-16 string.
   *
   * @return The converted string.
   */
  std::basic_string<js_char16_t> to_utf16_string() const;

  /**
   * Get a UTF-16 buffer.
   *
   * Valid only while the flusspferd::string is valid.
   *
   * @return The buffer.
   */
  js_char16_t const *data() const;

  /**
   * Create a substring object.
   *
   * @param start The index of the first character to include.
   * @param length The number of characters.
   * @return The substring.
   */
  string substr(size_t start, size_t length) const;

  /**
   * Concatenate two strings.
   *
   * @param a The first string.
   * @param b The second string.
   * @return The concatenated string.
   */
  static string concat(string const &a, string const &b);
};

/**
 * Compare two string%s for equality.
 *
 * @relates string
 */
bool operator==(string const &lhs, string const &rhs);

/**
 * Compare two string%s lexicographically.
 *
 * @return Whether @p lhs is lexicographically smaller than @p rhs.
 *
 * @relates string
 */
bool operator<(string const &lhs, string const &rhs);

template<>
struct detail::convert<string> {
  typedef to_value_helper<string> to_value;

  struct from_value {
    root_value root;

    string perform(value const &v) {
      string s = v.to_string();
      root = s;
      return s;
    }
  };
};

template<>
struct detail::convert<char const *> {
  typedef to_value_helper<char const *, string> to_value;

  struct from_value {
    root_value root;

    char const *perform(value const &v) {
      string s = v.to_string();
      root = s;
      return s.c_str();
    }
  };
};

template<>
struct detail::convert<std::string> {
  typedef to_value_helper<std::string, string> to_value;

  struct from_value {
    std::string perform(value const &v) {
      return v.to_string().to_string();
    }
  };
};

template<>
struct detail::convert<std::basic_string<js_char16_t> > {
  typedef std::basic_string<js_char16_t> string_t;

  typedef to_value_helper<string_t, string> to_value;

  struct from_value {
    string_t perform(value const &v) {
      return v.to_string().to_utf16_string();
    }
  };
};

}

#endif /* FLUSSPFERD_STRING_HPP */
