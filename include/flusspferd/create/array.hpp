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

#ifndef FLUSSPFERD_CREATE_ARRAY_HPP
#define FLUSSPFERD_CREATE_ARRAY_HPP

#include "../create.hpp"
#include "../array.hpp"

namespace flusspferd {

namespace detail {
  array create_length_array(std::size_t length);

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

}

}

#endif
