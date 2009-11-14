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

#ifndef FLUSSPFERD_CREATE_FUNCTION_HPP
#define FLUSSPFERD_CREATE_FUNCTION_HPP

#include "../create.hpp"
#include "../string.hpp"
#include "../local_root_scope.hpp"
#include <boost/static_assert.hpp>

namespace flusspferd {

/**
 * Tag class for flusspferd::create.
 *
 * Used as an alternative to flusspferd::function when the first C++
 * parameter should be the Javascript 'this' object.
 *
 * @ingroup create
 */
struct method {};

#ifndef IN_DOXYGEN

namespace detail {
  function create_source_function(
    flusspferd::string const &name,
    std::vector<flusspferd::string> const &argnames,
    flusspferd::string const &body,
    flusspferd::string const &file,
    unsigned line);

  function create_native_function(native_function_base *ptr);

  template<bool Method, typename ArgPack>
  struct create_function_helper {
    typedef function result_type;

    typedef typename boost::parameter::value_type<
        ArgPack,
        param::tag::argument_names,
        std::vector<flusspferd::string>
      >::type ArgumentsRange;

    static result_type create(ArgPack const &arg, flusspferd::string const &func) {
      local_root_scope scope;
      ArgumentsRange const &r =
        arg[param::_argument_names | std::vector<flusspferd::string>()];
      std::vector<flusspferd::string> arg_names(boost::begin(r), boost::end(r));
     
      return create_source_function(
        flusspferd::string(arg[param::_name | flusspferd::string()]),
        arg_names,
        func,
        arg[param::_file | flusspferd::string()],
        arg[param::_line | 0]);
    }

    static result_type create(ArgPack const &arg, char const *func) {
      return create(arg, flusspferd::string(func));
    }

    static result_type create(
      ArgPack const &arg,
      boost::function<void (call_context&)> const &fun)
    {
      local_root_scope scope;
      return create_native_function(
        new native_function<void>(
          fun,
          arg[param::_arity | 0],
          string(arg[param::_name | string()]).to_string()));
    }

    template<typename T>
    static result_type create(
      ArgPack const &arg,
      boost::function<T> const &fun)
    {
      return create_native_function(
        new native_function<T, Method>(
          fun,
          string(arg[param::_name | string()]).to_string()
        ));
    }

    template<typename T>
    static result_type create(
      ArgPack const &arg,
      T *fnptr,
      typename boost::enable_if<boost::is_function<T> >::type* = 0)
    {
      return create(arg, boost::function<T>(fnptr));
    }

    template<typename T>
    static result_type create(
      ArgPack const &arg,
      void (T::*memfnptr)(call_context &))
    {
      BOOST_STATIC_ASSERT(Method);
      return create_native_function(
        new native_member_function<void, T>(
          memfnptr,
          arg[param::_arity | 0],
          string(arg[param::_name | string()]).to_string()));
    }

    template<typename R, typename T>
    static result_type create(
      ArgPack const &arg,
      R T::*memfnptr)
    {
      BOOST_STATIC_ASSERT(Method);
      return create_native_function(
        new native_member_function<R, T>(
          memfnptr,
          string(arg[param::_name | string()]).to_string()));
    }
  };

  template<bool Method>
  struct create_function_traits {
    typedef function result_type;

    typedef boost::parameter::parameters<
        param::tag::name,
        param::tag::function,
        param::tag::argument_names,
        param::tag::file,
        param::tag::line,
        param::tag::signature,
        param::tag::arity,
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
    static
    typename boost::enable_if<
      boost::is_same<
        typename boost::parameter::binding<
          ArgPack, param::tag::signature, void
        >::type,
        void
      >,
      result_type
    >::type
    create(ArgPack const &arg) {
      typedef create_function_helper<Method, ArgPack> helper;
      return helper::create(arg, arg[param::_function]);
    }

    template<typename ArgPack>
    static
    typename boost::disable_if<
      boost::is_same<
        typename boost::parameter::binding<
          ArgPack, param::tag::signature, void
        >::type,
        void
      >,
      result_type
    >::type
    create(ArgPack const &arg) {
      typedef create_function_helper<Method, ArgPack> helper;
      typedef typename boost::parameter::value_type<
          ArgPack,
          param::tag::signature
        >::type::parameter sig;
      return helper::create(arg, boost::function<sig>(arg[param::_function]));
    }
  };

  template<>
  struct create_traits<function, void>
  : create_function_traits<false>
  {};

  template<>
  struct create_traits<method, void>
  : create_function_traits<true>
  {};
}

#endif //IN_DOXYGEN

}

#endif
