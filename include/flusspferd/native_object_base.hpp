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

#ifndef FLUSSPFERD_NATIVE_OBJECT_BASE_HPP
#define FLUSSPFERD_NATIVE_OBJECT_BASE_HPP

#include "object.hpp"
#include "convert.hpp"
#include "function_adapter.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_member_function_pointer.hpp>
#include <memory>
#include <functional>

namespace flusspferd {

struct call_context;
class tracer;

class native_object_base : public object, private boost::noncopyable {
protected:
  native_object_base();

  virtual void post_initialize();

protected:
  typedef void (native_object_base::*native_method_type)(call_context &);
  typedef boost::function<void (call_context &)> callback_type;

protected:
  void add_native_method(std::string const &name, unsigned arity = 0);

protected:
  void register_native_method(
    std::string const &name, native_method_type method);

  void register_native_method_cb(
    std::string const &name, callback_type const &cb);

  template<typename T>
  void register_native_method(
    std::string const &name, void (T::*method)(call_context&))
  {
    register_native_method(name, native_method_type(method));
  }

  template<typename T, typename X>
  void register_native_method_cb(std::string const &name, X const &cb) {
    boost::function<T> fun(cb);
    function_adapter<T> adapter(fun);
    register_native_method_cb(name, adapter);
  }

  template<typename R, typename T>
  void register_native_method(std::string const &name, R T::*f) {
    function_adapter_memfn<R, T> adapter(f);
    register_native_method_cb(name, adapter);
  }

protected:
  static function create_native_method(
    std::string const &name, unsigned arity = 0);

protected:
  virtual void call_native_method(std::string const &name, call_context &);

  virtual void trace(tracer &);

public:
  virtual ~native_object_base() = 0;

  object get_object() {
    return *static_cast<object*>(this);
  }

  static native_object_base *get_native(object const &o);

private:
  void invalid_method(call_context &);

private:
  object do_create_object(object const &proto);

  friend object create_native_object(native_object_base *, object const &);

public:
  class impl;
  boost::scoped_ptr<impl> p;

  friend class impl;
};

template<typename T>
struct detail::convert_ptr<T, native_object_base> {
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
      return native_object_base::get_native(v.get_object());
    }
  };
};

namespace detail {

template<typename T, typename O>
struct convert_ptr<
  T, O,
  typename boost::enable_if<
    typename boost::is_base_of<native_object_base, O>::type
  >::type
>
{
  typedef typename convert_ptr<native_object_base>::to_value to_value;

  struct from_value {
    typename convert_ptr<native_object_base>::from_value base;

    T *perform(value const &v) {
      return &dynamic_cast<T&>(base.perform(v));
    }
  };
};

}

}

#endif
