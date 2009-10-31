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

#ifndef FLUSSPFERD_CREATE_HPP
#define FLUSSPFERD_CREATE_HPP

#ifndef PREPROC_DEBUG
#include "object.hpp"
#include "function.hpp"
#include "native_function.hpp"
#include "local_root_scope.hpp"
#include <boost/type_traits/is_function.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/range.hpp>
#include <boost/parameter/parameters.hpp>
#include <boost/parameter/keyword.hpp>
#include <boost/parameter/name.hpp>
#include <boost/parameter/binding.hpp>
#include <boost/type_traits.hpp>
#endif
#include "detail/limit.hpp"
#include <boost/preprocessor.hpp>
#include <boost/parameter/config.hpp>

namespace flusspferd {

class native_object_base;
class function;
class native_function_base;

/**
 * @name Creating functions
 * @addtogroup create_function
 */
//@{

namespace detail {

/**
 * Create a new native function.
 *
 * @p ptr will be <code>delete</code>d by Flusspferd.
 *
 * @param ptr The native function object.
 * @return The new function.
 */
function create_native_function(native_function_base *ptr);

/**
 * Create a new native function as method of an object.
 *
 * The new method of object @p o will have the name @c ptr->name().
 *
 * @param o The object to add the method to.
 * @param ptr The native function object.
 * @return The new method.
 */
function create_native_function(object const &o, native_function_base *ptr);

}

#define FLUSSPFERD_FN_CREATE_NATIVE_FUNCTION(z, n_args, d) \
  template< \
    typename T \
    BOOST_PP_ENUM_TRAILING_PARAMS(n_args, typename T) \
  > \
  object old_create_native_functor_function( \
    object const &o \
    BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n_args, T, const & param), \
    typename boost::enable_if_c<!boost::is_function<T>::value>::type * = 0 \
  ) { \
    return detail::create_native_function(o, new T(BOOST_PP_ENUM_PARAMS(n_args, param))); \
  } \
  /* */

BOOST_PP_REPEAT(
  BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT),
  FLUSSPFERD_FN_CREATE_NATIVE_FUNCTION,
  ~
)

/**
 * Create a new native method of an object.
 *
 * @param o The object to add the method to.
 * @param name The method name.
 * @param fn The functor to call.
 * @param arity The function arity.
 * @return The new function.
 */
inline function create_native_function(
  object const &o,
  std::string const &name,
  boost::function<void (call_context &)> const &fn,
  unsigned arity = 0)
{
  return old_create_native_functor_function<native_function<void> >(
      o, fn, arity, name);
}

/**
 * Create a new native method of an object.
 *
 * @param T The function signature to use.
 * @param o The object to add the method to.
 * @param name The function name.
 * @param fn The functor to call.
 * @return The new function.
 */
template<typename T>
function create_native_function(
  object const &o,
  std::string const &name,
  boost::function<T> const &fn)
{
  return old_create_native_functor_function<native_function<T,false> >(o, fn, name);
}

/**
 * Create a new native method of an object.
 *
 * The first parameter passed will be 'this'.
 *
 * @param T The function signature to use.
 * @param o The object to add the method to.
 * @param name The function name.
 * @param fn The functor to call.
 * @return The new function.
 */
template<typename T>
function create_native_method(
  object const &o,
  std::string const &name,
  boost::function<T> const &fn)
{
  return old_create_native_functor_function<native_function<T,true> >(o, fn, name);
}

/**
 * Create a new native method of an object.
 *
 * @param o The object to add the method to.
 * @param name The method name.
 * @param fnptr The function to call (also determines the function signature).
 * @return The new method.
 */
template<typename T>
function create_native_function(
  object const &o,
  std::string const &name,
  T *fnptr,
  typename boost::enable_if_c<boost::is_function<T>::value>::type* =0)
{
  return create_native_function<T>(o, name, boost::function<T>(fnptr));
}

/**
 * Create a new native method of an object.
 *
 * The first parameter passed will be 'this'.
 *
 * @param o The object to add the method to.
 * @param name The method name.
 * @param fnptr The function to call (also determines the function signature).
 * @return The new method.
 */
template<typename T>
function create_native_method(
  object const &o,
  std::string const &name,
  T *fnptr,
  typename boost::enable_if_c<boost::is_function<T>::value>::type* =0)
{
  return create_native_method<T>(o, name, boost::function<T>(fnptr));
}

/**
 * Create a new native method of an object.
 *
 * @param o The object to add the method to.
 * @param name The method name.
 * @param memfnptr The member function to call.
 * @return The new function.
 */
template<typename T>
function create_native_method(
  object const &o,
  std::string const &name,
  void (T::*memfnptr)(call_context &),
  unsigned arity = 0)
{
  return old_create_native_functor_function<native_member_function<void, T> >(
    o, memfnptr, arity, name);
}

/**
 * Create a new native method of an object.
 *
 * @param o The object to add the method to.
 * @param name The method name.
 * @param memfnptr The member function to call (also determines the function
 *                 signature).
 * @return The new function.
 */
template<typename R, typename T>
function create_native_method(
  object const &o,
  std::string const &name,
  R T::*memfnptr)
{
  return old_create_native_functor_function<native_member_function<R, T> >(
    o, memfnptr, name);
}

//@}

namespace param {
  BOOST_PARAMETER_NAME(container)
  BOOST_PARAMETER_NAME(name)
  BOOST_PARAMETER_NAME(attributes)

  BOOST_PARAMETER_NAME(length)
  BOOST_PARAMETER_NAME(contents)
  BOOST_PARAMETER_NAME(prototype)
  BOOST_PARAMETER_NAME(parent)

  BOOST_PARAMETER_NAME(argument_names)
  BOOST_PARAMETER_NAME(function)
  BOOST_PARAMETER_NAME(file)
  BOOST_PARAMETER_NAME(line)
  BOOST_PARAMETER_NAME(signature)
  BOOST_PARAMETER_NAME(arity)

  BOOST_PARAMETER_NAME(arguments)

  /* For passing types. Like this: _param = param::type<int>() */
  template<typename T>
  struct type {
    typedef T parameter;
  };
}

namespace detail {
  template<typename Class>
  struct new_functor {
    template<typename>
    struct result {
      typedef Class &type;
    };

    Class &operator()() {
      return *new Class;
    }

#define FLUSSPFERD_NEW_FUNCTOR_INVOKE(z, n, d) \
    template< \
      BOOST_PP_ENUM_PARAMS(n, typename T) \
    > \
    Class &operator()( \
      BOOST_PP_ENUM_BINARY_PARAMS(n, T, &x) \
    ) \
    { \
      return *new Class(BOOST_PP_ENUM_PARAMS(n, x)); \
    } \
    /* */

    BOOST_PP_REPEAT_FROM_TO(
      1,
      BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT),
      FLUSSPFERD_NEW_FUNCTOR_INVOKE,
      ~)

#undef FLUSSPFERD_NEW_FUNCTOR_INVOKE
  };

  template<typename Class, typename Cond = void>
  struct create_traits;

  typedef param::tag::container container_spec;
  typedef param::tag::name name_spec;
  typedef param::tag::attributes attributes_spec;

  template<typename Class, typename ArgPack>
  typename boost::enable_if<
    boost::is_same<
      typename boost::parameter::binding<
        ArgPack,
        param::tag::container,
        void
      >::type,
      void
    >,
    typename create_traits<Class>::result_type
  >::type
  create_helper(ArgPack const &arg) {
    return create_traits<Class>::create(arg);
  }

  template<typename Class, typename ArgPack>
  typename boost::disable_if<
    boost::is_same<
      typename boost::parameter::binding<
        ArgPack,
        param::tag::container,
        void
      >::type,
      void
    >,
    typename create_traits<Class>::result_type
  >::type
  create_helper(ArgPack const &arg) {
    typedef create_traits<Class> traits;
    local_root_scope scope;

    typename traits::result_type result = traits::create(arg);

    object container(arg[param::_container]);
    container.define_property(
      arg[param::_name],
      result,
      arg[param::_attributes | dont_enumerate]);

    return result;
  }
}

template<typename Class>
typename detail::create_traits<Class>::result_type
create()
{
  return detail::create_traits<Class>::create();
}

#define FLUSSPFERD_CREATE(z, n, d) \
  template< \
    typename Class, \
    BOOST_PP_ENUM_PARAMS(n, typename T) \
  > \
  typename detail::create_traits<Class>::result_type \
  create( \
    BOOST_PP_ENUM_BINARY_PARAMS(n, T, const &x), \
    typename detail::create_traits<Class>::parameters::template match<T0>::type kw = typename detail::create_traits<Class>::parameters()) \
  { \
    typedef detail::create_traits<Class> traits; \
    return detail::create_helper<Class>(kw(BOOST_PP_ENUM_PARAMS(n, x))); \
  } \
  /* */

BOOST_PP_REPEAT_FROM_TO(
  1,
  BOOST_PP_INC(BOOST_PARAMETER_MAX_ARITY),
  FLUSSPFERD_CREATE,
  ~)

#undef FLUSSPFERD_CREATE

}

#endif
