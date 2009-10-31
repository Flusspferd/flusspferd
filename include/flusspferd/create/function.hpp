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

namespace flusspferd {

struct method {}; /* tag */

namespace detail {
  function create_source_function(
    flusspferd::string const &name,
    std::vector<flusspferd::string> const &argnames,
    flusspferd::string const &body,
    flusspferd::string const &file,
    unsigned line);

  template<bool Method>
  struct create_function_traits {
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

  template<>
  struct create_traits<function, void>
  : create_function_traits<false>
  {};

  template<>
  struct create_traits<method, void>
  : create_function_traits<true>
  {};
}

}

#endif
