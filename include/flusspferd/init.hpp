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

#ifndef FLUSSPFERD_INIT_HPP
#define FLUSSPFERD_INIT_HPP

#include "flusspferd/context.hpp"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace flusspferd {

class context;
class object;

/**
 * Manage the current context and the initialisation of the Javascript engine.
 *
 * @ingroup contexts
 */
class init : boost::noncopyable {
  init();

  class impl;
  boost::scoped_ptr<impl> p;

public:
#ifndef IN_DOXYGEN
  ~init();

  struct detail;
  friend struct init::detail;
#endif

  /**
   * Set the current context to @p c.
   *
   * @param c The new current context.
   * @return The old current context.
   */
  context enter_current_context(context const &c);

  /**
   * Unset the current context.
   *
   * Sets the current context to an invalid context if the current context is
   * @p c or the current context is already invalid.
   *
   * @param c The context to unset.
   * @return Whether the unsetting was successful.
   */
  bool leave_current_context(context const &c);

  /**
   * Get the current context.
   *
   * @return The current context.
   */
  context &current_context();

  /**
   * Initialize the Javascript engine if needed. Works as a singleton.
   *
   * @return The global #init object (singleton).
   */
  static init &initialize();
};

/**
 * Set the current context to @p c.
 *
 * @see init::enter_current_context
 * @ingroup contexts
 */
inline context enter_current_context(context const &c) {
  return init::initialize().enter_current_context(c);
}

/**
 * Unset the current context.
 *
 * @see init::leave_current_context
 * @ingroup contexts
 */
inline bool leave_current_context(context const &c) {
  return init::initialize().leave_current_context(c);
}

/**
 * Get the current context.
 *
 * @see init::current_context
 * @ingroup contexts
 */
inline context &current_context() {
  return init::initialize().current_context();
}

/**
 * Get the global object of the current context.
 *
 * @see current_context, context::global
 */
inline object global() {
  return current_context().global();
}

/**
 * Get the global object of the current context.
 *
 * @see current_context, context::scope_chain
 */
inline object scope_chain() {
  return current_context().scope_chain();
}

/**
 * Run the garbage collector on the current context.
 *
 * @see current_context, context::gc
 *
 * @ingroup gc
 */
inline void gc() {
  return current_context().gc();
}

/**
 * Get a prototype from the current context's prototype registry.
 *
 * @see current_context, context::prototype(void)
 */
template<typename T>
object prototype() {
  return current_context().prototype<T>();
}

/**
 * Get a constructor from the current context's constructor registry.
 *
 * @see current_context, context::constructor(void)
 */
template<typename T>
object constructor() {
  return current_context().constructor<T>();
}

/**
 * Get a prototype from the current context's prototype registry.
 *
 * @see current_context, context::prototype(const std::string &name) const
 */
inline object prototype(std::string const &ID) {
  return current_context().prototype(ID);
}

/**
 * Get a constructor from the current context's constructor registry.
 *
 * @see current_context, context::constructor(const std::string & name) const
 */
inline object constructor(std::string const &ID) {
  return current_context().constructor(ID);
}

}

#endif /* FLUSSPFERD_INIT_HPP */
