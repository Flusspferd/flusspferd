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

#ifndef FLUSSPFERD_NATIVE_FUNCTION_BASE_HPP
#define FLUSSPFERD_NATIVE_FUNCTION_BASE_HPP

#include "init.hpp"
#include "function.hpp"
#include "convert.hpp"
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_member_function_pointer.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <string>

namespace flusspferd {

struct call_context;
#ifndef IN_DOXYGEN
class tracer;
#endif

namespace detail {
  function create_native_function(native_function_base *);
}

/**
 * Native function base.
 *
 * @ingroup functions
 */
class native_function_base : public function, private boost::noncopyable {
public:
  native_function_base(function const &obj);
  virtual ~native_function_base();

  static native_function_base *get_native(object const &o);

  /**
   * Virtual method invoked whenever the object has to be traced.
   *
   * Default implementation: stub.
   *
   * @see native_object_base::trace
   * @see @ref gc
   */
  virtual void trace(tracer &trc);

  /**
   * Static function for determining the arity in flusspferd::create.
   *
   * Should be overwritten if you want to have an automatically determined
   * arity, which will have precedence over any user-supplied arity.
   *
   * @return If an arity is determined, <tt>boost::optional<unsigned>(Arity)</tt>,
   *         otherwise an empty <tt>boost::optional<unsigned>()</tt>.
   */
  static boost::optional<unsigned> determine_arity() {
    return boost::optional<unsigned>();
  }

protected:
  virtual void call(call_context &) = 0;

public:
#ifndef IN_DOXYGEN
  static function create_function(
      unsigned arity, std::string const &name);

  void load_into(function const &obj);
#endif

private:
  class impl;
  boost::scoped_ptr<impl> p;

  friend class impl;
};

template<typename T>
T &cast_to_derived(native_function_base &o) {
  T *ptr = dynamic_cast<T*>(&o);
  if (!ptr)
    throw exception("Could not convert native object to derived type");
  return *ptr;
}

/**
 * Gets @p o as native function of class @p T. If @p o is not a function, not
 * native or not of class @p T then an exception will be thrown.
 *
 * @param o object to check
 * @see is_native
 * @ingroup classes
 */
template<typename T>
typename boost::enable_if<
  typename boost::is_base_of<native_function_base, T>::type,
  T &>
::type
get_native(object const &o) {
  return flusspferd::cast_to_derived<T>(*native_function_base::get_native(o));
}

template<typename T>
bool is_derived(native_function_base &o) {
  return dynamic_cast<T*>(&o);
}

/**
 * Checks if @p o is a native function of class @p T.
 *
 * @code
flusspferd::object o = v.get_object();
if (flusspferd::is_native<my_fun>(o) {
  my_fun &b = flusspferd::get_native<my_fun>(o);
}
@endcode
 *
 * @param o object to check
 * @see get_native
 * @ingroup classes
 */
template<typename T>
typename boost::enable_if<
  typename boost::is_base_of<native_function_base, T>::type,
  bool>
::type
is_native(object const &o) {
  try {
    get_native<T>(o);
    return true;
  } catch (exception&) {
    return false;
  }
}

template<typename T>
struct detail::convert_ptr<T, native_function_base> {
  struct to_value {
    value perform(T *ptr) {
      if (!ptr)
        return object();
      return *static_cast<object const *>(ptr);
    }
  };

  struct from_value {
    T *perform(value const &v) {
      if (!v.is_object())
        throw exception("Value is no object");
      return native_function_base::get_native(v.get_object());
    }
  };
};

}

#endif
