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

#ifndef FLUSSPFERD_CREATE_OBJECT_HPP
#define FLUSSPFERD_CREATE_OBJECT_HPP

#include "../create.hpp"

namespace flusspferd {

namespace detail {
  object create_object(object const &prototype, object const &parent);

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
}

}

#endif
