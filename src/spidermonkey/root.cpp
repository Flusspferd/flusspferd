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

#include "flusspferd/root.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/function.hpp"
#include "flusspferd/implementation/context.hpp"
#include "flusspferd/implementation/value.hpp"
#include <js/jsapi.h>

namespace flusspferd { namespace detail {

template<typename T>
root<T>::root(T const &o)
: T(o), ctx(get_current_context())
{
  JSBool status;

  status = JS_AddRoot(
    Impl::get_context(ctx),
    T::get_gcptr());

  if (status == JS_FALSE) {
    throw exception("Cannot root Javascript value");
  }
}

template<typename T>
root<T>::~root() {
  JS_RemoveRoot(
    Impl::get_context(ctx),
    T::get_gcptr());
}

template class root<value>;
template class root<object>;
template class root<string>;
template class root<function>;

}}
