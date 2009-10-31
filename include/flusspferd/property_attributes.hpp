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

#ifndef FLUSSPFERD_PROPERTY_ATTRIBUTES
#define FLUSSPFERD_PROPERTY_ATTRIBUTES

#include <boost/optional.hpp>

namespace flusspferd {

class function;

/*
 * Property flags.
 *
 * @ingroup property_types
 */
enum property_flag {
  /// The property is normal.
  no_property_flag = 0U,

  /// The property is not enumerable.
  dont_enumerate = 1U,

  /// The property is read-only.
  read_only_property = 2U,

  /// The property can not be deleted.
  permanent_property = 4U,

  /// The property's attributes are shared between objects.
  shared_property = 8U,

  /// The property is both permanent and shared.
  permanent_shared_property = 12U
};

/**
 * Combine (union).
 *
 * @relates property_flag
 */
inline property_flag operator|(property_flag a, property_flag b) {
  return static_cast<property_flag>(unsigned(a) | unsigned(b));
}

/**
 * Combine (intersection).
 */
inline property_flag operator&(property_flag a, property_flag b) {
  return static_cast<property_flag>(unsigned(a) & unsigned(b));
}

/**
 * Invert.
 */
inline property_flag operator~(property_flag x) {
  return static_cast<property_flag>(~unsigned(x));
}

/**
 * A property's attributes: flags, getters and setters.
 *
 * @ingroup property_types
 */
struct property_attributes {
  /// The property's flags.
  property_flag flags;

  /**
   *  The property's getter.
   *
   *  A function returning the property's value.
   */
  boost::optional<function const &> getter;

  /**
   * The property's setter.
   *
   * A function taking the property's value.
   */
  boost::optional<function const &> setter;

  /// Construct default attributes.
  property_attributes();

  /**
   * Construct property attributes.
   *
   * @param flags The flags.
   * @param getter The getter.
   * @param setter The setter.
   */
  property_attributes(property_flag flags, 
    boost::optional<function const &> getter = boost::none,
    boost::optional<function const &> setter = boost::none);
};

}

#endif
