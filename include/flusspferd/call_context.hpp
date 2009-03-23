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

#ifndef FLUSSPFERD_CALL_CONTEXT_HPP
#define FLUSSPFERD_CALL_CONTEXT_HPP

#include "object.hpp"
#include "arguments.hpp"
#include "value.hpp"

namespace flusspferd {

class native_object_base;

/**
 * Context information about a %function call.
 */
struct call_context {
  /// Default constructor.
  call_context() : self_native(0) {}

  /**
   * The 'this' object.
   */
  object self;

  /**
   * The 'this' object as a pointer to native_object_base. Might be @c NULL
   * even if @p self is a native object.
   */
  native_object_base *self_native;

  /**
   * The %function parameters.
   */
  arguments arg;

  /**
   * The %function's result.
   */
  value result;

  /**
   * The %function object.
   */
  object function;
};

}

#endif
