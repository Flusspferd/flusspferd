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

#ifndef FLUSSPFERD_NATIVE_FUNCTION_HPP
#define FLUSSPFERD_NATIVE_FUNCTION_HPP

#include "native_function_base.hpp"
#include "function_adapter.hpp"
#include <boost/function.hpp>

namespace flusspferd {

template<class T = void>
class native_function : public native_function_base {
public:
  typedef boost::function<T> callback_type;

  native_function(
      callback_type const &cb,
      std::string const &name = std::string()
    )
    : native_function_base(function_adapter<T>::arity, name), adapter(cb)
  {}

private:
  void call(call_context &x) {
    adapter(x);
  }

  function_adapter<T> adapter;
};

template<>
class native_function<void> : public native_function_base {
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

}

#endif
