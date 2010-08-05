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

#ifndef FLUSSPFERD_ROOT_VALUE_HPP
#define FLUSSPFERD_ROOT_VALUE_HPP

#include <boost/noncopyable.hpp>

namespace flusspferd {

class context;
class value;
class object;
class string;
class function;
class array;

namespace detail {

/**
 * Keeps a Javascript value, object or anything in a GC %root scope.
 *
 * If the GC thing could not be rooted, an exception will be thrown.
 *
 * A root object can be used transparently as the type it roots. For example:
 *
 * @verbatim
class root_example {
private:
  root_object root; // root_object is root<object>
public:
  object root_object(object to_root) {
    // The object to_root will now be protected from GC as long as the
    // root_example instance stays in scope. When it goes out, the root will
    // be removed.
    root = to_root;

    // We can treat the root_object as a plain object
    return root;
  }
}
@endverbatim
 *
 * The above example would work just the same if to_root as a string, value,
 * function or an array.
 *
 * @see root_value, root_object, root_string, root_object, root_array,
 *      local_root_scope
 *
 * @ingroup gc
 */
template<class T>
class root : public T, private boost::noncopyable {
public:
  /**
   * Construct the %root scope.
   *
   * @param x The initial value.
   */
  explicit root(T const &x = T());

  /// Destructor.
  ~root();

  /// Assignment.
  root &operator=(T const &o) {
    T::operator=(o);
    return *this;
  }

private:
  void **get_gcptr();
};

}

/**
 * @addtogroup gc
 */
//@{

/// Javascript root scope for a flusspferd::value.
typedef detail::root<value> root_value;

/// Javascript root scope for a flusspferd::object.
typedef detail::root<object> root_object;

/// Javascript root scope for a flusspferd::string.
typedef detail::root<string> root_string;

/// Javascript root scope for a flusspferd::array.
typedef detail::root<array> root_array;

//@}

}

#endif
