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

#ifndef FLUSSPFERD_NATIVE_FUNCTION_BASE_HPP
#define FLUSSPFERD_NATIVE_FUNCTION_BASE_HPP

#include "detail/api.hpp"
#include "init.hpp"
#include "function.hpp"
#include "convert.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <string>

namespace flusspferd {

struct call_context;

/**
 * Native function base.
 *
 * @ingroup functions
 */
class FLUSSPFERD_API native_function_base : public function, private boost::noncopyable {
public:
  native_function_base(unsigned arity = 0);
  native_function_base(unsigned arity, std::string const &name);
  virtual ~native_function_base();

  static native_function_base *get_native(object const &o);

protected:
  virtual void call(call_context &) = 0;

private:
  function create_function();

  friend FLUSSPFERD_API function create_native_function(native_function_base *);

private:
  class impl;
  boost::scoped_ptr<impl> p;

  friend class impl;
};

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
