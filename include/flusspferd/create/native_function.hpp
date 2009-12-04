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
  template<typename Class>
  struct create_traits<
    Class,
    typename boost::enable_if<
      boost::is_base_of<native_function_base, Class>
    >::type
  >
  {
    typedef Class &result_type;

    typedef boost::parameter::parameters<
      param::tag::arguments,
      param::tag::arity,
      name_spec,
      container_spec,
      attributes_spec
    > parameters;

    typedef boost::parameter::parameters<
      boost::parameter::required<param::tag::name>,
      param::tag::arguments,
      param::tag::arity,
      attributes_spec
    > create_on_parameters;

    static result_type create()
    {
      boost::optional<unsigned> determined_arity(Class::determine_arity());

      unsigned arity = 0u;
      if (determined_arity)
        arity = determined_arity.get();

      root_function fun(
          native_function_base::create_function(arity, std::string()));
      return *new Class(fun);
    }

    template<typename ArgPack>
    static result_type
    create(ArgPack const &args) {
      boost::optional<unsigned> determined_arity(Class::determine_arity());

      unsigned arity;
      if (determined_arity)
        arity = determined_arity.get();
      else
        arity = args[param::_arity | 0u];

      flusspferd::root_string name(args[param::_name | flusspferd::string()]);
      root_function fun(
          native_function_base::create_function(
              arity,
              name.to_string()));

      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::arguments,
          VECTOR0
        >::type input_arguments_type;

      input_arguments_type input_arguments(
          args[param::_arguments | VECTOR0()]);

      typedef
        boost::fusion::vector1<function>
        obj_seq_type;

      obj_seq_type obj_seq(fun);

      typedef boost::fusion::joint_view<
          obj_seq_type,
          input_arguments_type
        > full_arguments_type;

      full_arguments_type full_arguments(obj_seq, input_arguments);

      return boost::fusion::invoke(new_functor<Class>(), full_arguments);
    }
  };
}

#undef VECTOR0

#endif //IN_DOXYGEN

}

#endif
