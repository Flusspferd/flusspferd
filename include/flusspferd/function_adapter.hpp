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

#ifndef PREPROC_DEBUG
#include "convert.hpp"
#include "call_context.hpp"
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/function.hpp>
#endif
#include <boost/preprocessor.hpp>

#ifndef FLUSSPFERD_PARAM_LIMIT
#define FLUSSPFERD_PARAM_LIMIT 5
#endif

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

  p = from_value.perform(x.self);

  return p;
}

template<
  typename T,
  typename R = typename T::result_type,
  std::size_t A = T::arity,
  typename Condition = void>
struct function_adapter;

template<typename T, typename R>
struct function_adapter<
  T, R, 1,
  typename boost::enable_if<
    typename is_native_object_type<typename T::arg1_type>::type
  >::type
>
{
  typename convert<R>::to_value to_value;

  void action(T const &function, call_context &x) {
    native_object_base *obj = get_native_object_parameter(x);
    x.result = to_value.perform(
      function(ptr_to_native_object_type<typename T::arg1_type>::get(obj)));
  }
};

template<typename T>
struct function_adapter<
  T, void, 1,
  typename boost::enable_if<
    typename is_native_object_type<typename T::arg1_type>::type
  >::type
>
{
  void action(T const &function, call_context &x) {
    native_object_base *obj = get_native_object_parameter(x);
    function(ptr_to_native_object_type<typename T::arg1_type>::get(obj));
  }
};

template<typename T, typename R>
struct function_adapter<
  T, R, 1,
  typename boost::enable_if<
    typename boost::is_convertible<typename T::arg1_type, object>::type
  >::type
>
{
  typename convert<R>::to_value to_value;

  void action(T const &function, call_context &x) {
    x.result = to_value.perform(function(x.self));
  }
};

template<typename T>
struct function_adapter<
  T, void, 1,
  typename boost::enable_if<
    typename boost::is_convertible<typename T::arg1_type, object>::type
  >::type
>
{
  void action(T const &function, call_context &x) {
    function(x.self);
  }
};

#define FLUSSPFERD_DECLARE_ARG_CONVERTER(z, i, T) \
  typename convert<typename T::BOOST_PP_CAT(BOOST_PP_CAT(arg, i), _type)>::from_value \
  BOOST_PP_CAT(BOOST_PP_CAT(arg, i), _from_value); \
  /**/

#define FLUSSPFERD_DECLARE_ARG_CONVERTERS(first, last, T) \
  BOOST_PP_REPEAT_FROM_TO( \
    first, \
    BOOST_PP_INC(last), \
    FLUSSPFERD_DECLARE_ARG_CONVERTER, \
    T) \
  /**/

#define FLUSSPFERD_CONVERT_ARG(z, i, x) \
  BOOST_PP_COMMA_IF(BOOST_PP_GREATER(i, 1)) \
  BOOST_PP_CAT(BOOST_PP_CAT(arg, i), _from_value) \
  .perform((x).arg[BOOST_PP_DEC(i)]) \
  /**/

#define FLUSSPFERD_CONVERT_ARGS(first, last, x) \
  BOOST_PP_REPEAT_FROM_TO( \
    first, \
    BOOST_PP_INC(last), \
    FLUSSPFERD_CONVERT_ARG, \
    x) \
  /**/

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER(z, n_args, d) \
  template<typename T, typename R, typename C> \
  struct function_adapter<T, R, n_args, C> { \
    typename convert<R>::to_value to_value; \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(1, n_args, T) \
    void action(T const &function, call_context &x) { \
      x.result = to_value.perform(FLUSSPFERD_CONVERT_ARGS(1, n_args, x)); \
    } \
  }; \
  /**/

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(n) \
  BOOST_PP_REPEAT_FROM_TO( \
    0, \
    n, \
    FLUSSPFERD_DECLARE_FUNCTION_ADAPTER, \
    ~ \
  ) \
  /**/

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_R_VOID(z, n_args, d) \
  template<typename T, typename C> \
  struct function_adapter<T, void, n_args, C> { \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(1, n_args, T) \
    void action(T const &function, call_context &x) { \
      function(FLUSSPFERD_CONVERT_ARGS(1, n_args, x)); \
    } \
  }; \
  /**/

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS_R_VOID(n) \
  BOOST_PP_REPEAT_FROM_TO( \
    0, \
    BOOST_PP_INC(n), \
    FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_R_VOID, \
    ~) \
  /**/

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(FLUSSPFERD_PARAM_LIMIT)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS_R_VOID(FLUSSPFERD_PARAM_LIMIT)

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
    detail::function_adapter<function_type> function_adapter_implementation;
    function_adapter_implementation.action(function, x);
  }

private:
  function_type function;
};

}

#endif
