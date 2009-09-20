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

#ifndef FLUSSPFERD_NATIVE_FUNCTION_HPP
#define FLUSSPFERD_NATIVE_FUNCTION_HPP

#include "native_function_base.hpp"
#include "function_adapter.hpp"
#include <boost/function.hpp>

namespace flusspferd {

/**
 * Native function.
 *
 * @ingroup functions
 */
template<class T = void, bool Method = false>
class native_function : public native_function_base {
private:
  typedef function_adapter<T, Method> adapter_type;

public:
  typedef boost::function<T> callback_type;

  native_function(
      callback_type const &cb,
      std::string const &name = std::string()
    )
    : native_function_base(adapter_type::arity, name), adapter(cb)
  {}

private:
  void call(call_context &x) {
    adapter(x);
  }

  adapter_type adapter;
};

#ifndef IN_DOXYGEN
template<>
class native_function<void, false> : public native_function_base {
public:
  typedef boost::function<void (call_context &)> callback_type;

  native_function(
      callback_type const &cb,
      unsigned arity = 0,
      std::string const &name = std::string()
    )
    : native_function_base(arity, name), cb(cb)
  {}

private:
  void call(call_context &x) {
    cb(x);
  }

  callback_type cb;
};
#endif

/**
 * Native member function.
 *
 * @ingroup functions
 */
template<typename R, typename T>
class native_member_function : public native_function_base {
private:
  typedef function_adapter_memfn<R, T> adapter_type;

public:
  typedef R T::*callback_type;

  native_member_function(
      callback_type const &cb,
      std::string const &name = std::string()
    )
    : native_function_base(adapter_type::arity, name), adapter(cb)
  {}

private:
  void call(call_context &x) {
    adapter(x);
  }

  adapter_type adapter;
};

#ifndef IN_DOXYGEN
template<typename T>
class native_member_function<void, T> : public native_function_base {
public:
  typedef void (T::*callback_type)(call_context &);

  native_member_function(
      callback_type const &cb,
      unsigned arity = 0,
      std::string const &name = std::string()
    )
    : native_function_base(arity, name), cb(cb)
  {}

private:
  void call(call_context &x) {
    T *ptr;
    if (x.self_native)
      ptr = &flusspferd::cast_to_derived<T>(*x.self_native);
    else
      ptr = &flusspferd::get_native<T>(x.self);
    (ptr->*cb)(x);
  }

  callback_type cb;
};
#endif

}

#endif
