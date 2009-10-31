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

#ifndef FLUSSPFERD_CREATE_NATIVE_OBJECT_HPP
#define FLUSSPFERD_CREATE_NATIVE_OBJECT_HPP

#include "../create.hpp"
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/view/joint_view.hpp>
#include <boost/fusion/container/vector/vector10.hpp>

namespace flusspferd {

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
}

}

#endif
