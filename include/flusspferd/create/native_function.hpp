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

#ifndef FLUSSPFERD_CREATE_NATIVE_FUNCTION_HPP
#define FLUSSPFERD_CREATE_NATIVE_FUNCTION_HPP

#include "../create.hpp"
#include <boost/version.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/view/joint_view.hpp>
#include <boost/fusion/container/vector/vector10.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/mpl/eval_if.hpp>

namespace flusspferd {

#ifndef IN_DOXYGEN

#if BOOST_VERSION < 104100
#define VECTOR0 boost::fusion::vector0
#else
#define VECTOR0 boost::fusion::vector0<>
#endif

namespace detail {
  BOOST_MPL_HAS_XXX_TRAIT_DEF(ignore_name_arity)

  template<typename T>
  struct ignore_name_arity_test
  {
    typedef typename T::ignore_name_arity type;
  };

  template<typename T>
  struct ignore_name_arity
    : boost::mpl::eval_if<
        has_ignore_name_arity<T>,
        ignore_name_arity_test<T>,
        boost::mpl::false_
      >
  {};

  template<typename Class>
  struct create_traits<
    Class,
    typename boost::enable_if<
      boost::mpl::and_<
        boost::is_base_of<native_function_base, Class>,
        ignore_name_arity<Class>
      >
    >::type
  >
  {
    typedef function result_type;

    typedef boost::parameter::parameters<
      param::tag::arguments,
      name_spec,
      container_spec,
      attributes_spec
    > parameters;

    typedef typename ignore_name_arity<Class>::type ignore_name_arity_type;

    static result_type create()
    {
      return create_native_function(new Class);
    }

    template<typename ArgPack>
    static result_type create(ArgPack const &args) {
      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::arguments,
          VECTOR0
        >::type input_arguments_type;

      input_arguments_type input_arguments(
        args[param::_arguments | VECTOR0()]);

      Class &self = boost::fusion::invoke(new_functor<Class>(), input_arguments);

      return create_native_function(&self);
    }
  };

  template<typename Class>
  struct create_traits<
    Class,
    typename boost::enable_if<
      boost::mpl::and_<
        boost::is_base_of<native_function_base, Class>,
        boost::mpl::not_<ignore_name_arity<Class> >
      >
    >::type
  >
  {
    typedef function result_type;

    typedef boost::parameter::parameters<
      param::tag::arguments,
      name_spec,
      container_spec,
      attributes_spec
    > parameters;

    typedef typename ignore_name_arity<Class>::type ignore_name_arity_type;

    static result_type create(
        typename boost::disable_if<ignore_name_arity_type>::type * = 0)
    {
        return create_native_function(new Class(0U, std::string()));
    }

    template<typename ArgPack>
    static
    typename boost::disable_if<ignore_name_arity_type, result_type>::type
    create(ArgPack const &args) {
      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::arguments,
          VECTOR0
        >::type input_arguments_type;

      input_arguments_type input_arguments(
        args[param::_arguments | VECTOR0()]);

      typedef
        boost::fusion::vector2<unsigned, std::string>
        arity_name_seq_type;

      arity_name_seq_type arity_name_seq(
          args[param::_arity | 0u],
          std::string(args[param::_name | std::string()]));

      typedef boost::fusion::joint_view<
          arity_name_seq_type,
          input_arguments_type
        > full_arguments_type;

      full_arguments_type full_arguments(arity_name_seq, input_arguments);

      Class &self = boost::fusion::invoke(new_functor<Class>(), full_arguments);

      return create_native_function(&self);
    }
  };
}

#undef VECTOR0

#endif //IN_DOXYGEN

}

#endif
