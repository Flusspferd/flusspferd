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

#ifndef FLUSSPFERD_FUNCTION_ADAPTER_HPP
#define FLUSSPFERD_FUNCTION_ADAPTER_HPP

#ifndef PREPROC_DEBUG
#include "convert.hpp"
#include "call_context.hpp"
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/function.hpp>
#endif
#include "detail/limit.hpp"
#include <boost/preprocessor.hpp>

namespace flusspferd {

class native_object_base;

template<typename T>
T &cast_to_derived(native_object_base &);

#ifndef IN_DOXYGEN

namespace detail {

template<typename T>
struct is_native_object_type {
  typedef typename boost::remove_reference<T>::type T1;
  typedef typename boost::remove_pointer<T1>::type naked_type;

  typedef typename boost::is_base_of<native_object_base, naked_type>::type type;
};

template<typename T, typename Condition = void>
struct ptr_to_native_object_type {
  typedef typename is_native_object_type<T>::naked_type naked_type;

  static T get(native_object_base &self_native) {
    return cast_to_derived<naked_type>(self_native);
  }
};

template<typename T>
struct ptr_to_native_object_type<
    T, typename boost::enable_if<typename boost::is_pointer<T>::type>::type>
{
  typedef typename is_native_object_type<T>::naked_type naked_type;

  static T get(native_object_base &self_native) {
    return &cast_to_derived<naked_type>(self_native);
  }
};

native_object_base &get_native_object_parameter_ptr(call_context &x);

template<typename T>
typename T::arg1_type
get_native_object_parameter(call_context &x) {
  native_object_base &p = get_native_object_parameter_ptr(x);
  return ptr_to_native_object_type<typename T::arg1_type>::get(p);
}

template<typename T>
T get_native_object_parameter2(call_context &x) {
  native_object_base &p = get_native_object_parameter_ptr(x);
  return ptr_to_native_object_type<T>::get(p);
}

#define FLUSSPFERD_DECLARE_ARG_CONVERTER(z, i, T) \
  typename convert<typename T::BOOST_PP_CAT(BOOST_PP_CAT(arg, i), _type)>::from_value \
  BOOST_PP_CAT(BOOST_PP_CAT(arg, i), _from_value); \
  /* */

#define FLUSSPFERD_DECLARE_ARG_CONVERTERS(first, last, T) \
  BOOST_PP_REPEAT_FROM_TO( \
    first, \
    BOOST_PP_INC(last), \
    FLUSSPFERD_DECLARE_ARG_CONVERTER, \
    T) \
  /* */

#define FLUSSPFERD_CONVERT_ARG(z, i, offset) \
  BOOST_PP_COMMA_IF(BOOST_PP_GREATER(i, 1)) \
  BOOST_PP_CAT(BOOST_PP_CAT(arg, i), _from_value) \
  .perform((x).arg[BOOST_PP_SUB(BOOST_PP_DEC(i), offset)]) \
  /* */

#define FLUSSPFERD_CONVERT_ARGS(first, last, offset) \
  BOOST_PP_REPEAT_FROM_TO( \
    first, \
    BOOST_PP_INC(last), \
    FLUSSPFERD_CONVERT_ARG, \
    offset) \
  /* */

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_(z, n_args, d) \
  template<typename T, typename R, typename C> \
  struct function_adapter<T, false, R, n_args, C> { \
    typename convert<R>::to_value to_value; \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(1, n_args, T) \
    void action(T const &function, call_context &x) { \
      x.result = to_value.perform( \
        function(FLUSSPFERD_CONVERT_ARGS(1, n_args, 0))); \
    } \
    static std::size_t const arity = (n_args); \
  }; \
  /* */

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_R_VOID(z, n_args, d) \
  template<typename T, typename C> \
  struct function_adapter<T, false, void, n_args, C> { \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(1, n_args, T) \
    void action(T const &function, call_context &x) { \
      (void)x; \
      function(FLUSSPFERD_CONVERT_ARGS(1, n_args, 0)); \
    } \
    static std::size_t const arity = (n_args); \
  }; \
  /* */

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_NATIVE_OBJECT(z, n_args, d) \
  template<typename T, typename R> \
  struct function_adapter< \
    T, true, R, n_args, \
    typename boost::enable_if< \
      typename is_native_object_type<typename T::arg1_type>::type \
    >::type \
  > \
  { \
    typename convert<R>::to_value to_value; \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(2, n_args, T) \
    void action(T const &function, call_context &x) { \
      x.result = to_value.perform( \
        function( \
          get_native_object_parameter<T>(x) \
          FLUSSPFERD_CONVERT_ARGS(2, n_args, 1) \
        )); \
    } \
    static std::size_t const arity = BOOST_PP_DEC(n_args); \
  }; \
  /* */

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_NATIVE_OBJECT_R_VOID(z, n_args, d) \
  template<typename T> \
  struct function_adapter< \
    T, true, void, n_args, \
    typename boost::enable_if< \
      typename is_native_object_type<typename T::arg1_type>::type \
    >::type \
  > \
  { \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(2, n_args, T) \
    void action(T const &function, call_context &x) { \
      function( \
        get_native_object_parameter<T>(x) \
        FLUSSPFERD_CONVERT_ARGS(2, n_args, 1) \
      ); \
    } \
    static std::size_t const arity = BOOST_PP_DEC(n_args); \
  }; \
  /* */

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_OBJECT(z, n_args, d) \
  template<typename T, typename R> \
  struct function_adapter< \
    T, true, R, n_args, \
    typename boost::enable_if< \
      typename boost::is_convertible<object, typename T::arg1_type>::type \
    >::type \
  > \
  { \
    typename convert<R>::to_value to_value; \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(2, n_args, T) \
    void action(T const &function, call_context &x) { \
      x.result = to_value.perform(function( \
          x.self \
          FLUSSPFERD_CONVERT_ARGS(2, n_args, 1) \
        )); \
    } \
    static std::size_t const arity = BOOST_PP_DEC(n_args); \
  }; \
  /* */

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_OBJECT_R_VOID(z, n_args, d) \
  template<typename T> \
  struct function_adapter< \
    T, true, void, n_args, \
    typename boost::enable_if< \
      typename boost::is_convertible<object, typename T::arg1_type>::type \
    >::type \
  > \
  { \
    FLUSSPFERD_DECLARE_ARG_CONVERTERS(2, n_args, T) \
    void action(T const &function, call_context &x) { \
      function( \
        x.self \
        FLUSSPFERD_CONVERT_ARGS(2, n_args, 1) \
      ); \
    } \
    static std::size_t const arity = BOOST_PP_DEC(n_args); \
  }; \
  /* */

#define FLUSSPFERD_DECLARE_ARG_CONVERTER_MEMFN(z, i, d) \
  typename convert<BOOST_PP_CAT(P, i)>::from_value \
  BOOST_PP_CAT(BOOST_PP_CAT(arg, BOOST_PP_INC(i)), _from_value);

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_MEMFN(z, n_args, d) \
  template< \
    typename T, \
    typename R \
    BOOST_PP_ENUM_TRAILING_PARAMS(n_args, typename P) \
  > \
  struct function_adapter_memfn< \
    R (T::*)(BOOST_PP_ENUM_PARAMS(n_args, P)) \
  > \
  { \
    typename convert<R>::to_value to_value; \
    BOOST_PP_REPEAT(n_args, FLUSSPFERD_DECLARE_ARG_CONVERTER_MEMFN, ~) \
    template<typename F> \
    void action(F fun, call_context &x) { \
      x.result = to_value.perform( \
        (get_native_object_parameter2<T*>(x)->*fun) \
          (FLUSSPFERD_CONVERT_ARGS(1, n_args, 0))); \
    } \
    static std::size_t const arity = (n_args); \
  }; \
  /* */


#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTER_MEMFN_R_VOID(z, n_args, d) \
  template< \
    typename T \
    BOOST_PP_ENUM_TRAILING_PARAMS(n_args, typename P) \
  > \
  struct function_adapter_memfn< \
    void (T::*)(BOOST_PP_ENUM_PARAMS(n_args, P)) \
  > \
  { \
    BOOST_PP_REPEAT(n_args, FLUSSPFERD_DECLARE_ARG_CONVERTER_MEMFN, ~) \
    template<typename F> \
    void action(F fun, call_context &x) { \
      (get_native_object_parameter2<T*>(x)->*fun) \
          (FLUSSPFERD_CONVERT_ARGS(1, n_args, 0)); \
    } \
    static std::size_t const arity = (n_args); \
  }; \
  /* */

#define FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(suffix) \
  BOOST_PP_REPEAT_FROM_TO( \
    0, \
    BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT), \
    BOOST_PP_CAT(FLUSSPFERD_DECLARE_FUNCTION_ADAPTER, suffix), \
    ~) \
  /* */

template<
  typename Type,
  bool Method,
  typename ResultType = typename Type::result_type,
  std::size_t Arity = Type::arity,
  typename Condition = void>
struct function_adapter;

template<
  typename F
> struct function_adapter_memfn;

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_R_VOID)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_NATIVE_OBJECT)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_NATIVE_OBJECT_R_VOID)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_OBJECT)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_OBJECT_R_VOID)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_MEMFN)

FLUSSPFERD_DECLARE_FUNCTION_ADAPTERS(_MEMFN_R_VOID)

}

#endif

/**
 * Function adapter.
 *
 * @ingroup functions
 */
template<typename T, bool Method>
class function_adapter {
public:
  typedef T spec_type;
  typedef boost::function<spec_type> function_type;

private:
  typedef detail::function_adapter<function_type, Method> adapter_type;

public:
  function_adapter(function_type const &function)
    : function(function)
  {}

  void operator() (call_context &x) {
    adapter_type function_adapter_implementation;
    function_adapter_implementation.action(function, x);
  }

  static std::size_t const arity = adapter_type::arity;

private:
  function_type function;
};

/**
 * Function adapter.
 *
 * @ingroup functions
 */
template<typename R, typename T>
class function_adapter_memfn {
private:
  typedef R T::*funptr_type;

  typedef detail::function_adapter_memfn<funptr_type> adapter_type;

public:
  function_adapter_memfn(R T::*funptr)
    : funptr(funptr)
  {}

  void operator() (call_context &x) {
    adapter_type function_adapter_implementation_memfn;
    function_adapter_implementation_memfn.action(funptr, x);
  }

  static std::size_t const arity = adapter_type::arity;

private:
  R T::*funptr;
};

}

#include "native_object_base.hpp"

#endif
