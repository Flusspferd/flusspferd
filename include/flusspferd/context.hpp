// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

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

#ifndef FLUSSPFERD_CONTEXT_HPP
#define FLUSSPFERD_CONTEXT_HPP

#include "detail/api.hpp"
#include "object.hpp"
#include <boost/shared_ptr.hpp>
#include <string>

namespace flusspferd {

class value;
class object;
class native_object_base;

/**
 * Javascript %context.
 *
 * @ingroup contexts
 */
class FLUSSPFERD_PUBLIC_API context {
  class impl;
  boost::shared_ptr<impl> p;

  struct context_private;

public:
  /// Constructor for an empty, invalid context.
  context();

  /// Destructor.
  ~context();

#ifndef IN_DOXYGEN
  struct detail;
  friend struct detail;

  context(detail const&);
#endif

  /// Check if the context is valid.
  bool is_valid() const;

  /// Compare to another context for equality.
  bool operator==(context const &o) const {
    return p == o.p;
  }

  /// Create a new valid context.
  static context create();

  /// Get the global Javascript object of the context.
  object global();

  /// Get the Javascript scope chain object.
  object scope_chain();

  /**
   * Run the garbage collector.
   *
   * @see flusspferd::gc
   */
  void gc();

  /**
   * Add a prototype to the context's prototype registry.
   *
   * @param name The name and ID of the prototype.
   * @param proto The prototype.
   */
  void add_prototype(std::string const &name, object const &proto);

  /**
   * Get a prototype from the context's prototype registry.
   *
   * @param name The name and ID of the prototype.
   * @return The prototype.
   */
  object prototype(std::string const &name) const;

  /**
   * Add a prototype to the context's prototype registry.
   *
   * @param T     The class whose name (<code>T::class_info::full_name()</code>)
   *              is to be used.
   * @param proto The prototype.
   */
  template<typename T>
  void add_prototype(object const &proto) {
    add_prototype(T::class_info::full_name(), proto);
  }

  /**
   * Get a prototype from the context's prototype registry.
   *
   * @param T     The class whose name (<code>T::class_info::full_name()</code>)
   *              is to be used.
   * @return      The prototype.
   */
  template<typename T>
  object prototype() const {
    return prototype(T::class_info::full_name());
  }

  /**
   * Add a constructor to the context's constructor registry.
   *
   * @param name The name and ID of the constructor.
   * @param ctor The constructor.
   */
  void add_constructor(std::string const &name, object const &ctor);

  /**
   * Get a constructor to the context's constructor registry.
   *
   * @param name The name and ID of the constructor.
   * @return     The constructor.
   */
  object constructor(std::string const &name) const;

  /**
   * Add a constructor to the context's constructor registry.
   *
   * @param T     The class whose name (<code>T::class_info::full_name()</code>)
   *              is to be used.
   * @param ctor  The constructor.
   */
  template<typename T>
  void add_constructor(object const &ctor) {
    add_constructor(T::class_info::full_name(), ctor);
  }

  /**
   * Get a constructor from the context's constructor registry.
   *
   * @param T     The class whose name (<code>T::class_info::full_name()</code>)
   *              is to be used.
   * @return      The constructor.
   */
  template<typename T>
  object constructor() const {
    return constructor(T::class_info::full_name());
  }
};

template<>
inline object context::prototype<native_object_base>() const {
  return object();
}

/**
 * Compare two context%s for inequality.
 *
 * @relates context
 */
inline bool operator!=(context const &a, context const &b) {
  return !(a == b);
}

}

#endif /* FLUSSPFERD_CONTEXT_HPP */
