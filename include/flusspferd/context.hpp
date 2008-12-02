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

class context {
  class impl;
  boost::shared_ptr<impl> p;

  struct context_private;

public:
  struct detail;
  friend struct detail;

  context();
  context(detail const&);
  ~context();

  bool is_valid() const;

  bool operator==(context const &o) const {
    return p == o.p;
  }

  static context create();

  object global();

  object scope_chain();

  value evaluate(char const *source, std::size_t n,
                 char const *file = 0x0, unsigned int line = 0);

  value evaluateInScope(char const* source, std::size_t n,
                       char const* file, unsigned int line,
                       object scope);

  value evaluate(char const *source, char const *file = 0x0,
                 unsigned int line = 0);
  value evaluate(std::string const &source, char const *file = 0x0,
                 unsigned int line = 0);

  void gc();

  void add_prototype(std::string const &name, object const &proto);
  object get_prototype(std::string const &name) const;

  template<typename T>
  void add_prototype(object const &proto) {
    add_prototype(T::class_info::full_name(), proto);
  }

  template<typename T>
  object get_prototype() const {
    return get_prototype(T::class_info::full_name());
  }

  void add_constructor(std::string const &name, object const &ctor);
  object get_constructor(std::string const &name) const;

  template<typename T>
  void add_constructor(object const &ctor) {
    add_constructor(T::class_info::full_name(), ctor);
  }

  template<typename T>
  object get_constructor() const {
    return get_constructor(T::class_info::full_name());
  }
};

inline bool operator!=(context const &a, context const &b) {
  return !(a == b);
}

}

#endif /* FLUSSPFERD_CONTEXT_HPP */
