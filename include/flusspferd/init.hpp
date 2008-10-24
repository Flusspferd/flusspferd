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

namespace flusspferd { namespace js {
  class context;
  class object;

  class init : boost::noncopyable {
    init();
    ~init();

    class impl;
    boost::scoped_ptr<impl> p;

  public:
    struct detail;
    friend struct init::detail;

    // returns pointer to old context or null
    context enter_current_context(context const &c);
    // returns true if c was current context
    bool leave_current_context(context const &c);
    context &get_current_context();

    static init &initialize() {
      static init in;
      return in;
    }
  };

  inline context enter_current_context(context const &c) {
    return init::initialize().enter_current_context(c);
  }

  inline bool leave_current_context(context const &c) {
    return init::initialize().leave_current_context(c);
  }

  inline context &get_current_context() {
    return init::initialize().get_current_context();
  }

  object global();
}}

#endif /* FLUSSPFERD_INIT_HPP */
