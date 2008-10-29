// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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
#include <boost/type_traits/is_function.hpp>
#include <boost/utility/enable_if.hpp>
#endif
#include <boost/preprocessor.hpp>

#ifndef FLUSSPFERD_PARAM_LIMIT
#define FLUSSPFERD_PARAM_LIMIT 5
#endif

namespace flusspferd {

class native_object_base;
class function;
class native_function_base;

object create_object();
object create_array(unsigned int length = 0);
object create_native_object(native_object_base *ptr);

#define FLUSSPFERD_FN_CREATE_NATIVE_OBJECT(z, n_args, d) \
  template< \
    typename T \
    BOOST_PP_ENUM_TRAILING_PARAMS(n_args, typename T) \
  > \
  object create_native_object( \
    BOOST_PP_ENUM_BINARY_PARAMS(n_args, T, const & param) \
  ) { \
    return create_native_object(new T(BOOST_PP_ENUM_PARAMS(n_args, param))); \
  } \
  /**/

BOOST_PP_REPEAT(
  BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT),
  FLUSSPFERD_FN_CREATE_NATIVE_OBJECT,
  ~
)

function create_native_function(native_function_base *);

#define FLUSSPFERD_FN_CREATE_NATIVE_FUNCTION(z, n_args, d) \
  template< \
    typename T \
    BOOST_PP_ENUM_TRAILING_PARAMS(n_args, typename T) \
  > \
  object create_native_function( \
    BOOST_PP_ENUM_BINARY_PARAMS(n_args, T, const & param) \
    BOOST_PP_COMMA_IF(n_args) \
    typename boost::enable_if_c<!boost::is_function<T>::value>::type * = 0 \
  ) { \
    return create_native_function(new T(BOOST_PP_ENUM_PARAMS(n_args, param))); \
  } \
  /**/

BOOST_PP_REPEAT(
  BOOST_PP_INC(FLUSSPFERD_PARAM_LIMIT),
  FLUSSPFERD_FN_CREATE_NATIVE_FUNCTION,
  ~
)

inline function create_native_function(
  boost::function<void (call_context &)> const &fn)
{
  return create_native_function<native_function<void> >(fn);
}

template<typename T>
function create_native_function(
  boost::function<T> const &fn)
{
  return create_native_function<native_function<T> >(fn);
}

template<typename T>
function create_native_function(
  T *fnptr, typename boost::enable_if_c<boost::is_function<T>::value>::type* =0)
{
  return create_native_function<T>(boost::function<T>(fnptr));
}

}

#endif
