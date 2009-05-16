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

#ifndef FLUSSPFERD_PROPERTY_ITERATOR_HPP
#define FLUSSPFERD_PROPERTY_ITERATOR_HPP

#include "object.hpp"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/scoped_ptr.hpp>

namespace flusspferd {

/**
 * Property iterator.
 *
 * Uses boost::iterator_facade to support basic iterator operations.
 *
 * @ingroup property_types
 */
class property_iterator
  : public boost::iterator_facade<
      property_iterator,
      value const,
      boost::forward_traversal_tag
    >
{
public:
  /// Constructor for end()-iterator.
  property_iterator();

  /// Copy-constructor.
  property_iterator(property_iterator const &);

  /// Constructor for begin()-iterator.
  explicit property_iterator(object const &o);

  /// Destructor.
  ~property_iterator();

  /**
   * Swap with another iterator.
   *
   * @param o The iterator to swap with.
   */
  void swap(property_iterator &o) {
    p.swap(o.p);
  }

  /**
   * Assignment operator.
   *
   * @param o The iterator to assign.
   * @return *this
   */
  property_iterator &operator=(property_iterator const &o) {
    property_iterator(o).swap(*this);
    return *this;
  }

private:
  friend class boost::iterator_core_access;

  void increment();

  bool equal(property_iterator const &other) const;
  value const &dereference() const;

private:
  class impl;
  boost::scoped_ptr<impl> p;
};

}

#endif
