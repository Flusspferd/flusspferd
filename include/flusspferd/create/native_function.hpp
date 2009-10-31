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
#include <boost/fusion/functional/invocation/invoke.hpp>

namespace flusspferd {

namespace detail {
  template<typename Class>
  struct create_traits<
    Class,
    typename boost::enable_if<
      boost::is_base_of<native_function_base, Class>
    >::type
  >
  {
    typedef function result_type;

    typedef boost::parameter::parameters<
      param::tag::arguments,
      param::tag::prototype,
      name_spec,
      container_spec,
      attributes_spec
    > parameters;

    static result_type create() {
      return create_native_function(object(), new Class);
    }

    template<typename ArgPack>
    static result_type create(ArgPack const &args) {
      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::arguments,
          boost::fusion::vector0
        >::type input_arguments_type;

      input_arguments_type input_arguments(
        args[param::_arguments | boost::fusion::vector0()]);

      Class &self = boost::fusion::invoke(new_functor<Class>(), input_arguments);

      return create_native_function(args[param::_prototype | object()], &self);
    }
  };
}

}

#endif
