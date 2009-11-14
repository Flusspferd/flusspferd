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
#include "root.hpp"
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

namespace param {
#ifndef IN_DOXYGEN
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
#else
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _container;

  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _name;

  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _attributes;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _length;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _contents;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _prototype;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _parent;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _argument_names;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _function;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _file;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _line;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _signature;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _arity;
 
  /**
   * TODO
   *
   * @see flusspferd::create
   */
  unspecified_parameter_keyword_type _arguments;
#endif

  /**
   * For passing types as function parameters.
   *
   * @code
   _param = flusspferd::param::type<int>()
   @endcode
   */
  template<typename T>
  struct type {
    typedef T parameter;
  };
}

#ifndef IN_DOXYGEN

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

    object container_o(arg[param::_container]);
    root_object container(container_o);

    typename traits::result_type result(traits::create(arg));
    root_value root_result(result);

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

#else //IN_DOXYGEN

/**
 * Create an %object or %value of a type implied by @p Class.
 *
 * This %function takes a number of named parameters. Some named parameters can
 * be specified in a specific order, or even in any order, depending on @p Class.
 *
 * The return %value is always convertible to flusspferd::value.
 *
 * A few simple examples:
 * @code
// Create an empty object.
flusspferd::object o = flusspferd::create< flusspferd::object >();

// Create an empty object with a prototype.
o = flusspferd::create< flusspferd::object >(
      flusspferd::param::_prototype = proto);

// Create an array with three elements.
flusspferd::array a = flusspferd::create< flusspferd::array >(boost::assign::list_of(1)(2)(3));
@endcode
 *
 *
 * <dl>
 * <dt><b>Common parameters:</b></dt>
 * <dd>
 * <dl>
 * <dt>flusspferd::param::_container</dt>
 * <dd>The container object for storing the created %object.</dd>
 * <dt>flusspferd::param::_name</dt>
 * <dd>The name of the property to be used for storing the %object.</dd>
 * <dt>flusspferd::param::_attributes</dt>
 * <dd>Attributes to be used for creating the property.<br>
 *     <em>Default</em>: flusspferd::dont_enumerate</dt>
 * </dl>
 * <br>
 * </dd>
 * <dt><tt>Class</tt> = <b>object</b></dt>
 * <dd><em>Header</em>: flusspferd/create/object.hpp<br><br>
 * <dl>
 * <dt>flusspferd::param::_prototype</dt>
 * <dd>The prototype object to be used.<br>
 *     <em>Default</em>: <tt>Object.prototype</tt>
 * </dd>
 * <dt>flusspferd::param::_parent</dt>
 * <dd>The parent object to be used.<br>
 *     <em>Default</em>: Determined by the Javascript engine.
 * </dd>
 * </dl>
 * <br><em>Parameter order</em>:
 * flusspferd::param::_prototype, flusspferd::param::_parent
 * <br><br>
 * </dd>
 * <dt><tt>Class</tt> = <b>array</b></dt>
 * <dd>Header: flusspferd/create/array.hpp<br><br>
 * <dl>
 * <dt>flusspferd::param::_length</dt>
 * <dd>The length of the array. The elements will be <tt>undefined</tt>.<br>
 *     <em>Default</em>: <tt>0</tt>.
 * <dt>flusspferd::param::_contents</dt>
 * <dd>The contents of the array. Must be a valid
 *     <a href="http://www.boost.org/doc/libs/1_40_0/libs/range/index.html"
 *     >Boost.Range</a> (all STL containers fulfill this requirement). For
 *     static contents, we recommend
 *     <a href="http://www.boost.org/doc/libs/1_40_0/libs/assign/doc/index.html#list_of"
 *     >boost::assign::list_of</a>.</dd>
 * </dd>
 * </dl>
 * <br><em>Parameter order</em>:
 * First parameter is deduced: Integer values are flusspferd::param::_length,
 * otherwise it's flusspferd::param::_contents.
 * <br><br>
 * </dd>
 * <dt><tt>Class</tt> = <b>function</b> / <b>method</b></dt>
 * <dd>Header: flusspferd/create/function.hpp<br><br>
 * <dl>
 * <dt>flusspferd::param::_name</dt>
 * <dd>The function name.<br>
 *     <em>Default</em>: flusspferd::string()
 * </dd>
 * <dt>flusspferd::param::_function</dt>
 * <dd>The %function source code or %function pointer or member %function
 *     pointer (method only) or functor / boost::function.<br>
 *     <tt>void ()(call_context &)</tt> and <tt>void (T::*)(call_context &)</tt>
 *     are handled specially: The %function call information is all passed in
 *     the call_context parameter, as is the return %value slot.<br>
 *     <em>Default</em>: <tt>flusspferd::string()</tt>
 * </dd>
 * <dt>flusspferd::param::_argument_names</dt>
 * <dd>For Javascript source functions only.<br>
 *     The names of the %function %arguments. Must be compatible to Boost.Range.<br>
 *     <em>Default</em>: <tt>std::vector<flusspferd::string>()</tt>
 * </dd>
 * <dt>flusspferd::param::_file</dt>
 * <dd>For Javascript source functions only.<br>
 *     The name of the source file from which the function comes.<br>
 *     <em>Default</em>: <tt>flusspferd::string()</tt>
 * </dd>
 * <dt>flusspferd::param::_line</dt>
 * <dd>For Javascript source functions only.<br>
 *     The line number at which the function appears in the source file.<br>
 *     <em>Default</em>: <tt>0</tt>
 * </dd>
 * <dt>flusspferd::param::_signature</dt>
 * <dd>For native functions only.<br>
 *     The %function signature type. Use flusspferd::param::type<Signature>().
 *     The %function will be converted to boost::function<Signature> first.<br>
 *     <em>Default</em>: If unspecified, the signature will be determined
 *     automatically, if possible.
 * </dd>
 * <dt>flusspferd::param::_arity</dt>
 * <dd>The function arity.<br>
 *     Only used when the function arity can not otherwise be determined.<br>
 *     <em>Default</em>: <tt>0</tt>
 * </dd>
 * </dl>
 * <br>The difference between flusspferd::function and flusspferd::method is
 *     that, for flusspferd::method, the Javascript 'this' object is passed as
 *     the first C++ function parameter. For source functions, both have the
 *     same effect.
 * <br><br><em>Parameter order</em>: flusspferd::param::_name,
 *     flusspferd::param::_function, flusspferd::param::_argument_names,
 *     flusspferd::param::_file, flusspferd::param::_line,
 *     flusspferd::param::_signature, flusspferd::param::_arity.
 * <br><br>
 * </dd>
 * <dt><tt>Class</tt> = class derived from <b>native_object_base</b></dt>
 * <dd>Header: flusspferd/create/native_object.hpp</dd>
 * <dt><tt>Class</tt> = class derived from <b>native_function_base</b></dt>
 * <dd>Header: flusspferd/create/native_function.hpp</dd>
 * </dl>
 *
 * @ingroup create
 */
template<typename Class>
unspecified_type create(...);

#endif

}

#endif
