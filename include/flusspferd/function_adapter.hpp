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

#ifndef FLUSSPFERD_FUNCTION_ADAPTER_HPP
#define FLUSSPFERD_FUNCTION_ADAPTER_HPP

#include "convert.hpp"
#include "call_context.hpp"
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/function.hpp>

namespace flusspferd {

namespace detail {

template<typename T>
struct is_native_object_type {
  typedef typename boost::remove_reference<T>::type T2;
  typedef typename boost::remove_pointer<T2>::type native_object_type;

  typedef typename boost::is_convertible<T2, native_object_base>::type type;
};

template<typename T, typename Condition = void>
struct ptr_to_native_object_type {
  static T get(native_object_base *self_native) {
    return self_native;
  }
};

template<typename T>
struct ptr_to_native_object_type<
    T, typename boost::enable_if<typename boost::is_pointer<T>::type>::type>
{
  static T get(native_object_base *self_native) {
    return *self_native;
  }
};

native_object_base *
get_native_object_parameter(call_context &x) {
  native_object_base *p = x.self_native;

  if (p)
    return p;

  convert<native_object_base *>::from_value from_value;

  p = from_value.perform(x.arg[0]);

  return p;
}

template<
  typename T,
  typename R = typename T::result_type,
  std::size_t A = T::arity,
  typename Condition = void>
struct function_adapter;

template<typename T, typename R>
struct function_adapter<T, R, 0> {
  typename convert<R>::to_value to_value;

  void action(T const &function, call_context &x) {
    x.result = to_value.perform(function());
  }
};

template<typename T>
struct function_adapter<T, void, 0> {
  void action(T const &function, call_context &) {
    function();
  }
};

template<typename T, typename R>
struct function_adapter<
    T, R, 1, typename boost::enable_if<is_native_object_type<typename T::arg1_type> >::type>
{
  typename convert<R>::to_value to_value;

  void action(T const &function, call_context &x) {
    native_object_base *obj = get_native_object_parameter(x);
    x.result = to_value.perform(
        function(
          ptr_to_native_object_type<typename T::arg1_type>::get(obj)));
  }
};

}

template<typename T>
class function_adapter {
public:
  typedef T spec_type;
  typedef boost::function<spec_type> function_type;

  function_adapter(function_type const &function)
    : function(function)
  {}

  void operator() (call_context &x) {
    detail::function_adapter<function_type> helper;
    helper.action(function, x);
  }

private:
  function_type function;
};

}

#endif
