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

#ifndef FLUSSPFERD_FUNCTION_HPP
#define FLUSSPFERD_FUNCTION_HPP

#include "object.hpp"
#include "convert.hpp"
#include "spidermonkey/function.hpp"

namespace flusspferd {

class native_function_base;

/**
 * A Javascript function.
 *
 * @ingroup value_types
 * @ingroup functions
 */
class function : 
  public Impl::function_impl,
  public object
{
public:
  /**
   * Constructor.
   *
   * Creates a 'null' function (actually just an object).
   */
  function();

#ifndef IN_DOXYGEN
  function(Impl::function_impl const &f)
    : Impl::function_impl(f),
      object(Impl::function_impl::get_object())
  { }
#endif

  /**
   * Copy-constructor.
   *
   * @param o The function to copy.
   */
  function(function const &o)
    : Impl::function_impl(o),
      object(Impl::function_impl::get_object())
  { }

  /**
   * Conversion constructor.
   *
   * Tries to convert an object to a function.
   *
   * @param o The object to convert.
   */
  function(object const &o);

#ifndef IN_DOXYGEN
  function(Impl::object_impl const &o);
#endif

public:
  /**
   * Get the function arity.
   *
   * @return The arity.
   */
  std::size_t arity() const;

  /**
   * Get the function name.
   *
   * @return The name.
   */
  string name() const;
};

template<>
struct detail::convert<function> {
  typedef to_value_helper<function> to_value;

  struct from_value {
    root_value root;

    function perform(value const &v) {
      function f = function(v.to_object());
      root = object(f);
      return f;
    }
  };
};

}

#endif /* FLUSSPFERD_FUNCTION_HPP */
