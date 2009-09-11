// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

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

#ifndef FLUSSPFERD_ARRAY_HPP
#define FLUSSPFERD_ARRAY_HPP

#include "object.hpp"
#include <boost/utility/in_place_factory.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <cassert>

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

  class iterator
    : public boost::iterator_facade<
    iterator, value, boost::random_access_traversal_tag, value
    >
  {
    typedef boost::iterator_facade<
      iterator, value, boost::random_access_traversal_tag, value
      > parent_t;

    array const *a;
    std::size_t pos;
  public:
    explicit
    iterator(array const &a, std::size_t pos = 0) : a(&a),   pos(pos)  { }
    iterator(iterator const &i)                   : a(i.a), pos(i.pos) { }
    
    iterator &operator=(iterator const &o) {
      a = o.a;
      pos = o.pos;
      return *this;
    }

  private:
    friend class boost::iterator_core_access;

    void increment() {
      assert(a);
      ++pos;
      assert(pos <= a->size());
    }
    void decrement() {
      assert(a && pos > 0);
      --pos;
    }
    void advance(std::size_t n) {
      assert(a);
      pos += n;
      assert(pos <= a->size());
    }
    bool equal(iterator const &i) const {
      assert(a && i.a);
      return a == i.a && pos == i.pos;
    }
    parent_t::reference dereference() const {
      assert(a && pos < a->size());
      return a->get_element(pos);
    }
    parent_t::difference_type distance_to(iterator const &i) const {
      assert(a && a == i.a);
      typedef parent_t::difference_type diff_t;
      return static_cast<diff_t>(pos) - static_cast<diff_t>(i.pos);
    }
  };

  iterator begin() const { return iterator(*this); }
  iterator end()   const { return iterator(*this, size()); }
private:
  void check();
};

template<>
struct detail::convert<array> {
  typedef to_value_helper<array> to_value;

  struct from_value {
    boost::optional<root_array> root;

    array perform(value const &v) {
      root = boost::in_place(array(v.to_object()));
      return root.get();
    }
  };
};

}

#endif
