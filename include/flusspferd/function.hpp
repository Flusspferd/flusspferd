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

#ifndef FLUSSPFERD_FUNCTION_HPP
#define FLUSSPFERD_FUNCTION_HPP

#include "object.hpp"
#include "implementation/function.hpp"

namespace flusspferd {

class native_function_base;

class function : 
  public Impl::function_impl,
  public object
{
public:
  function();

  function(Impl::function_impl const &f)
    : Impl::function_impl(f),
      object(Impl::function_impl::get_object())
  { }

  function(function const &o)
    : Impl::function_impl(o),
      object(Impl::function_impl::get_object())
  { }

  function(object const &o);

  function(Impl::object_impl const &o);

public:
  static function create_native(native_function_base *);

public:
  std::size_t arity() const;
  string name() const;
};

}

#endif /* FLUSSPFERD_FUNCTION_HPP */
