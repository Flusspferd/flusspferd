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
#include "array.hpp"
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
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/view/joint_view.hpp>
#include <boost/fusion/container/vector/vector10.hpp>
#include <boost/type_traits.hpp>
#endif
#include "detail/limit.hpp"
#include <boost/preprocessor.hpp>
#include <boost/parameter/config.hpp>

namespace flusspferd {

class native_object_base;
class function;
class native_function_base;

#ifndef IN_DOXYGEN
namespace detail {

object create_native_object(
  object const &proto, object const &parent);
object create_native_enumerable_object(
  object const &proto, object const &parent);

template<typename T>
object generic_create_native_object(
  object proto,
  object const &parent,
  typename boost::disable_if<
    typename T::class_info::custom_enumerate
  >::type * = 0)
{
  if (proto.is_null())
    proto = current_context().prototype<T>();
  return detail::create_native_object(proto, parent);
}

template<typename T>
object generic_create_native_object(
  object proto,
  object const &parent,
  typename boost::enable_if<
    typename T::class_info::custom_enumerate
  >::type * = 0)
{
  if (proto.is_null())
    proto = current_context().prototype<T>();
  return detail::create_native_enumerable_object(proto, parent);
}

}
#endif

#ifndef IN_DOXYGEN

#define FLUSSPFERD_FN_CREATE_NATIVE_OBJECT(z, n_args, d) \
  template< \
    typename T \
    BOOST_PP_ENUM_TRAILING_PARAMS(n_args, typename T) \
  > \
  T& \
  create_native_object( \
    object proto \
    BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n_args, T, const & param) \
  ) { \
    local_root_scope scope; \
    object obj = detail::generic_create_native_object<T>(proto, object()); \
    return *(new T(obj BOOST_PP_ENUM_TRAILING_PARAMS(n_args, param))); \
  } \
  /* */

BOOST_PP_REPEAT(
  BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT),
  FLUSSPFERD_FN_CREATE_NATIVE_OBJECT,
  ~
)

#else

/**
 * Create a new native object of type @p T.
 *
 * @param T The type of the object's class.
 * @param proto The prototype to be used. If @p null, the class' default
 *          prototype will be used.
 * @param ... The parameters to the constructor of @p T.
 * @return The new object.
 */
template<typename T>
T &create_native_object(object const &proto, ...);

#endif
//@}

/**
 * @name Creating functions
 * @addtogroup create_function
 */
//@{

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

#ifndef IN_DOXYGEN

#define FLUSSPFERD_FN_CREATE_NATIVE_FUNCTION(z, n_args, d) \
  template< \
    typename T \
    BOOST_PP_ENUM_TRAILING_PARAMS(n_args, typename T) \
  > \
  object create_native_functor_function( \
    object const &o \
    BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(n_args, T, const & param), \
    typename boost::enable_if_c<!boost::is_function<T>::value>::type * = 0 \
  ) { \
    return create_native_function(o, new T(BOOST_PP_ENUM_PARAMS(n_args, param))); \
  } \
  /* */

BOOST_PP_REPEAT(
  BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT),
  FLUSSPFERD_FN_CREATE_NATIVE_FUNCTION,
  ~
)

#else

/**
 * Create a new native function of type @p F as method of an object.
 *
 * @p F must inherit from #native_function_base.
 *
 * The new method of object @p o will have the name @c ptr->name().
 *
 * @param F The functor type.
 * @param o The object to add the method to.
 * @param ... The parameters to pass to the constructor of @p F.
 * @return The new method.
 */
template<typename F>
object create_native_functor_function(object const &o, ...);

#endif

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
  return create_native_functor_function<native_function<void> >(
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
  return create_native_functor_function<native_function<T,false> >(o, fn, name);
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
  return create_native_functor_function<native_function<T,true> >(o, fn, name);
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
  return create_native_functor_function<native_member_function<void, T> >(
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
  return create_native_functor_function<native_member_function<R, T> >(
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
  BOOST_PARAMETER_NAME(source)
  BOOST_PARAMETER_NAME(file)
  BOOST_PARAMETER_NAME(line)

  BOOST_PARAMETER_NAME(arguments)
}

namespace detail {
  object create_object(object const &prototype, object const &parent);
  array create_length_array(std::size_t length);
  function create_source_function(
    flusspferd::string const &name,
    std::vector<flusspferd::string> const &argnames,
    flusspferd::string const &body,
    flusspferd::string const &file,
    unsigned line);

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

  template<>
  struct create_traits<flusspferd::object, void> {
    typedef flusspferd::object result_type;

    typedef boost::parameter::parameters<
        param::tag::prototype,
        param::tag::parent,
        container_spec,
        name_spec,
        attributes_spec
      > parameters;

    static result_type create() {
      return create_object(object(), object());
    }

    template<typename ArgPack>
    static result_type create(ArgPack const &arg) {
      return create_object(
        arg[param::_prototype | object()],
        arg[param::_parent | object()]);
    }
  };

  template<>
  struct create_traits<flusspferd::array, void> {
    typedef flusspferd::array result_type;

    typedef boost::parameter::parameters<
        boost::parameter::optional<
          boost::parameter::deduced<param::tag::length>,
          boost::is_integral<boost::mpl::_>
        >,
        boost::parameter::optional<
          boost::parameter::deduced<param::tag::contents>,
          boost::mpl::not_<boost::is_integral<boost::mpl::_> >
        >,
        container_spec,
        name_spec,
        attributes_spec
      >
      parameters;

    static flusspferd::array create() {
      return create_length_array(0);
    }

    template<typename ArgPack>
    static 
    typename boost::enable_if<
      boost::is_same<
        typename boost::parameter::binding<
          ArgPack,
          param::tag::contents,
          void
        >::type,
        void
      >,
      flusspferd::array
    >::type
    create(ArgPack const &arg) {
      return create_length_array(arg[param::_length | 0]);
    }

    template<typename ArgPack>
    static
    typename boost::disable_if<
      boost::is_same<
        typename boost::parameter::binding<
          ArgPack,
          param::tag::contents,
          void
        >::type,
        void
      >,
      flusspferd::array
    >::type
    create(ArgPack const &arg) {
      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::contents
        >::type
        Range;

      typedef typename boost::range_iterator<Range>::type iterator;

      Range const &r = arg[param::_contents];

      local_root_scope scope;

      array arr = create_length_array(0);

      iterator first = boost::begin(r);
      iterator last = boost::end(r);

      for (iterator it = first; it != last; ++it)
        arr.push(*it);

      return arr;
    }
  };

  template<>
  struct create_traits<function, void> {
    typedef function result_type;

    typedef boost::parameter::parameters<
        param::tag::name,
        param::tag::source,
        param::tag::argument_names,
        param::tag::file,
        param::tag::line,
        container_spec,
        attributes_spec
      > parameters;

    static result_type create() {
      flusspferd::root_string empty((flusspferd::string()));
      return create_source_function(
        empty,
        std::vector<flusspferd::string>(),
        empty,
        empty,
        0);
    }

    template<typename ArgPack>
    static result_type create(ArgPack const &arg) {
      local_root_scope scope;
      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::argument_names,
          std::vector<flusspferd::string>
        >::type Range;
      Range const &r = arg[param::_argument_names | std::vector<flusspferd::string>()];
      std::vector<flusspferd::string> arg_names(boost::begin(r), boost::end(r));
     
      return create_source_function(
        flusspferd::string(arg[param::_name | flusspferd::string()]),
        arg_names,
        arg[param::_source],
        arg[param::_file | flusspferd::string()],
        arg[param::_line | 0]);
    }
  };

  template<typename Class>
  struct create_traits<
    Class,
    typename boost::enable_if<
      boost::is_base_of<native_object_base, Class>
    >::type
  >
  {
    typedef Class &result_type;

    typedef boost::parameter::parameters<
        param::tag::arguments,
        param::tag::prototype,
        param::tag::parent,
        name_spec,
        container_spec,
        attributes_spec
      > parameters;

    static result_type create() {
      root_object obj((
          detail::generic_create_native_object<Class>(
            object(),
            object()
          )
        ));
      return *new Class(obj);
    }

    template<typename ArgPack>
    static result_type create(ArgPack const &args) {
      root_object obj((
          detail::generic_create_native_object<Class>(
            args[param::_prototype | object()],
            args[param::_parent | object()]
          )
        ));

      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::arguments,
          boost::fusion::vector0
        >::type input_arguments_type;

      input_arguments_type input_arguments(
        args[param::_arguments | boost::fusion::vector0()]);

      boost::fusion::vector1<object> obj_seq(obj);

      typedef boost::fusion::joint_view<
          boost::fusion::vector1<object>,
          input_arguments_type
        > full_arguments_type;

      full_arguments_type full_arguments(obj_seq, input_arguments);

      return boost::fusion::invoke(new_functor<Class>(), full_arguments);
    }
  };

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
