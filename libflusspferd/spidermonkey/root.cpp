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

#include "flusspferd/root.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/array.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include "flusspferd/spidermonkey/value.hpp"
#include <js/jsapi.h>

namespace flusspferd { namespace detail {
#ifdef FLUSSPFERD_JS_IS_JAEGERMONKEY
template<typename T>
root<T>::root(T const &o)
  : T(o)
{
  JSBool status = JS_AddGCThingRoot(
    Impl::current_context(),
    T::get_gcptr());

  if (status == JS_FALSE) {
    throw exception("Cannot root Javascript value");
  }
}

template<typename T>
root<T>::~root() {
  JS_RemoveGCThingRoot(
    Impl::current_context(),
    T::get_gcptr());
}

// value
template<>
root<value>::root(value const &o)
  : value(o)
{
  JSBool status = JS_AddValueRoot(
    Impl::current_context(),
    Impl::value_impl::getp());

  if (status == JS_FALSE) {
    throw exception("Cannot root Javascript value");
  }
}

template<>
root<value>::~root() {
  JS_RemoveValueRoot(
    Impl::current_context(),
    Impl::value_impl::getp());
}
#else
template<typename T>
root<T>::root(T const &o)
  : T(o)
{
  JSBool status = JS_AddRoot(
    Impl::current_context(),
    T::get_gcptr());

  if (status == JS_FALSE) {
    throw exception("Cannot root Javascript value");
  }
}

template<typename T>
root<T>::~root() {
  JS_RemoveRoot(
    Impl::current_context(),
    T::get_gcptr());
}
#endif

template class root<value>;
template class root<object>;
template class root<string>;
template class root<array>;
}}
