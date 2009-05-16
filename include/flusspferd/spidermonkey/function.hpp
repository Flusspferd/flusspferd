// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_SPIDERMONKEY_FUNCTION_HPP
#define FLUSSPFERD_SPIDERMONKEY_FUNCTION_HPP

#include <js/jsapi.h>

namespace flusspferd {

class object;

#ifndef IN_DOXYGEN

namespace Impl {

class function_impl {
  JSFunction *func;

protected:
  JSFunction *get() { return func; }
  JSFunction *get_const() const { return func; }
  void set(JSFunction *f) { func = f; }

  function_impl() : func(0x0) { }
  function_impl(JSFunction *f) : func(f) { }

  object get_object();

  friend JSFunction *get_function(function_impl &f);
  friend function_impl wrap_function(JSFunction *f);
};

inline JSFunction *get_function(function_impl &f) {
  return f.get();
}

inline function_impl wrap_function(JSFunction *f) {
  return function_impl(f);
}

}

#endif

}

#endif /* FLUSSPFERD_SPIDERMONKEY_FUNCTION_HPP */
