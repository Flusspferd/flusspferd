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

#ifndef FLUSSPFERD_CREATE_ON_HPP
#define FLUSSPFERD_CREATE_ON_HPP

#include "create.hpp"
#include "root.hpp"
#include <boost/noncopyable.hpp>

namespace flusspferd {

/**
 * Create multiple values / objects / stuff on a container.
 *
 * It works like flusspferd::create, except that the name parameter is always
 * the first.
 *
 * @code
 flusspferd::create_on(container)
     .create<function>("funName", &my_function)
     .create<object>("objName", my_prototype)
     // ... 
     ;
@endcode
 *
 * @ingroup create
 * @see flusspferd::create
 */
class create_on : boost::noncopyable {
public:
  /**
   * Constructor.
   *
   * @param container The container to put the newly created objects on.
   */
  create_on(object const &container)
    : container(container)
  { }

#ifndef IN_DOXYGEN
#define FLUSSPFERD_CREATE_ON_CREATE(z, n, d) \
  template< \
    typename Class, \
    BOOST_PP_ENUM_PARAMS(n, typename T) \
  > \
  create_on const & \
  create( \
    BOOST_PP_ENUM_BINARY_PARAMS(n, T, const &x) \
    , \
    typename detail::create_traits<Class>::create_on_parameters:: \
    template match< \
      BOOST_PP_ENUM_PARAMS(n, T) \
    >::type kw = \
    typename detail::create_traits<Class>::create_on_parameters() \
  ) \
  const \
  { \
    detail::create_helper<Class>(( \
          kw(BOOST_PP_ENUM_PARAMS(n, x)), \
          param::_container = this->container \
        )); \
    return *this; \
  } \
  /* */

  BOOST_PP_REPEAT_FROM_TO(
    1,
    BOOST_PP_INC(BOOST_PARAMETER_MAX_ARITY),
    FLUSSPFERD_CREATE_ON_CREATE,
    ~)

#undef FLUSSPFERD_CREATE_ON_CREATE
#else
  /**
   * Create a value of type Class on the container.
   *
   * Like flusspferd::create, but it does not take a container argument and the
   * name argument is always first and required.
   */
  template<typename Class>
  create_on const &create(...) const;
#endif

private:
  root_object container;
};


}

#endif
