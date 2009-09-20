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

#ifndef FLUSSPFERD_ARGUMENTS_HPP
#define FLUSSPFERD_ARGUMENTS_HPP

#include "spidermonkey/arguments.hpp"
#include "root.hpp"
#include "value.hpp"
#include <boost/shared_ptr.hpp>
#include <vector>

namespace flusspferd {

/**
 * Holds the %arguments for a Javascript %function call.
 *
 * @ingroup functions
 */
class arguments : public Impl::arguments_impl {
private:
  std::vector<boost::shared_ptr<root_value> > roots;

public:
  /// An empty arguments object.
  arguments() { }

#ifndef IN_DOXYGEN
  arguments(Impl::arguments_impl const &a)
    : Impl::arguments_impl(a)
  { }
#endif

  /**
   * An arguments object filled with the elements of a vector.
   *
   * The parameters will not be rooted.
   *
   * @param v The vector to fill the object with.
   */
  arguments(std::vector<value> const &v);

  /// The number of arguments.
  std::size_t size() const;

  /// Whether the object is empty (no arguments).
  bool empty() const;

  /**
   * Access an element from the arguments object by index.
   *
   * The element will be bound to the contents of the arguments object and
   * therefore changes will be reflected in the arguments object, too.
   *
   * If there is no argument with index i, flusspferd::value%() will be
   * returned.
   *
   * @param i The index of the argument to be accessed.
   */
  value operator[](std::size_t i);

  /**
   * Add a value to the back of the arguments list.
   *
   * The value will not be rooted.
   *
   * @param v The element to be added.
   */
  void push_back(value const &v);

  /**
   * Add a value to the back of the arguments list, rooting it.
   *
   * The value will be explicitly rooted.
   *
   * @param v The element to be added.
   */
  void push_root(value const &v);

  /**
   * Access the first argument.
   *
   * The element will be bound to the contents of the arguments object.
   *
   * If obj is empty, flusspferd::value%() will be returned.
   *
   * obj.front() is equivalent to obj[0].
   *
   * @see #operator[]
   */
  value front();

  /**
   * Access the last argument.
   *
   * The element will be bound to the contents of the arguments object.
   *
   * If obj is empty, flusspferd::value%() will be returned.
   *
   * obj.back() is equivalent to obj[obj.size() - 1] if obj is not empty.
   *
   * @see #operator[]
   */
  value back();

  /// An iterator class for flusspferd::arguments.
  class iterator : public Impl::arguments_impl::iterator_impl {
  public:
#ifndef IN_DOXYGEN
    iterator(Impl::arguments_impl::iterator_impl const &i)
      : Impl::arguments_impl::iterator_impl(i)
    { }
#endif

    /// Increment (pre-fix).
    iterator &operator++();

    /// Increment (post-fix).
    iterator operator++(int) {
      iterator tmp(*this);
      ++*this;
      return tmp;
    }

    /// Dereference.
    value operator*() const;

  public:
#ifndef IN_DOXYGEN
    bool equals(iterator const &b) const;
#endif
  };

  /// Return an iterator to the beginning of the arguments list.
  iterator begin();

  /// Return an iterator to the end of the arguments list.
  iterator end();
};

/**
 * Check two arguments::iterator%s for equality.
 *
 * @param lhs The first iterator to compare.
 * @param rhs The second iterator to compare.
 *
 * @relates flusspferd::arguments::iterator
 */
inline bool operator==(
  arguments::iterator const &lhs, arguments::iterator const &rhs)
{
  return lhs.equals(rhs);
}

/**
 * Check two arguments::iterator%s for inequality.
 *
 * @param lhs The first iterator to compare.
 * @param rhs The second iterator to compare.
 *
 * @relates flusspferd::arguments::iterator
 */
inline bool operator!=(arguments::iterator const &lhs,
                       arguments::iterator const &rhs)
{
  return !(lhs == rhs);
}

}

#endif /* FLUSSPFERD_ARGUMENTS_HPP */
