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

#include "flusspferd/js/root_value.hpp"
#include "flusspferd/js/init.hpp"
#include "flusspferd/js/exception.hpp"
#include "flusspferd/js/spidermonkey/context.hpp"
#include "flusspferd/js/spidermonkey/value.hpp"
#include <js/jsapi.h>

using namespace flusspferd::js;

root_value::root_value(value const &o)
: value(o), ctx(get_current_context())
{
  JSBool status;
  
  status = JS_AddRoot(
    Impl::get_context(ctx),
    Impl::get_jsvalp(*this));

  if (status == JS_FALSE) {
    throw exception("Cannot root Javascript value");
  }
}

root_value::~root_value() {
  JS_RemoveRoot(
    Impl::get_context(ctx),
    Impl::get_jsvalp(*this));
}
