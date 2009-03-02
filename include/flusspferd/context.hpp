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

#ifndef FLUSSPFERD_CONTEXT_HPP
#define FLUSSPFERD_CONTEXT_HPP

#include "object.hpp"
#include <boost/shared_ptr.hpp>
#include <string>

namespace flusspferd {

class value;
class object;

/**
 * Javascript %context.
 */
class context {
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
   * Evaluate Javascript code.
   *
   * Uses the global object as scope.
   *
   * @param source The source code.
   * @param n The length of the source code in bytes.
   * @param file The file name to use.
   * @param line The initial line number.
   */
  value evaluate(char const *source, std::size_t n,
                 char const *file = 0x0, unsigned int line = 0);

  /**
   * Evaluate Javascript code in a scope.
   *
   * @param source The source code.
   * @param n The length of the source code in bytes.
   * @param file The file name to use.
   * @param line The initial line number.
   * @param scope The scope 
   */
  value evaluate_in_scope(char const* source, std::size_t n,
                       char const* file, unsigned int line,
                       object const &scope);

  /**
   * Evaluate Javascript code.
   *
   * Uses the global object as context.
   *
   * @param source The source code.
   * @param file The file name to use.
   * @param line The initial line number.
   */
  value evaluate(char const *source, char const *file = 0x0,
                 unsigned int line = 0);

  /**
   * Evaluate Javascript code.
   *
   * Uses the global object as context.
   *
   * @param source The source code.
   * @param file The file name to use.
   * @param line The initial line number.
   */
  value evaluate(std::string const &source, char const *file = 0x0,
                 unsigned int line = 0);

  /**
   * Execute a Javascript file.
   *
   * @param file The path to the file.
   * @param scope The scope to use.
   */
  value execute(char const *file, object const &scope = object());

  /// Run the garbage collector.
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
