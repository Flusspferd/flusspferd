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

#ifndef FLUSSPFERD_INIT_HPP
#define FLUSSPFERD_INIT_HPP

#include "flusspferd/context.hpp"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace flusspferd {

class context;
class object;

class init : boost::noncopyable {
  init();

  class impl;
  boost::scoped_ptr<impl> p;

public:
  ~init();

  struct detail;
  friend struct init::detail;

  context enter_current_context(context const &c);
  bool leave_current_context(context const &c);

  context &current_context();

  static init &initialize();
};

inline context enter_current_context(context const &c) {
  return init::initialize().enter_current_context(c);
}

inline bool leave_current_context(context const &c) {
  return init::initialize().leave_current_context(c);
}

inline context &current_context() {
  return init::initialize().current_context();
}

inline object global() {
  return current_context().global();
}

inline void gc() {
  return current_context().gc();
}

template<typename T>
object prototype() {
  return current_context().prototype<T>();
}

template<typename T>
object constructor() {
  return current_context().constructor<T>();
}

inline object prototype(std::string const &ID) {
  return current_context().prototype(ID);
}

inline object constructor(std::string const &ID) {
  return current_context().constructor(ID);
}

}

#endif /* FLUSSPFERD_INIT_HPP */
