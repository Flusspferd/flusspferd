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

#ifndef FLUSSPFERD_CURRENT_CONTEXT_SCOPE_HPP
#define FLUSSPFERD_CURRENT_CONTEXT_SCOPE_HPP

#include "flusspferd/init.hpp"
#include "flusspferd/context.hpp"

namespace flusspferd {

class context;

/**
 * Load a context while in scope.
 *
 * @ingroup contexts
 */
class current_context_scope {
private:
  context c;
  context old;

public:
  /**
   * Constructor.
   *
   * @param c The context to load.
   */
  current_context_scope(context const &c)
      : c(c)
  {
    old = enter_current_context(this->c);
  }

  /**
   * Destructor.
   */
  ~current_context_scope() {
    if(leave_current_context(c) && old.is_valid())
      enter_current_context(old);
  }
};

}

#endif /* FLUSSPFERD_CURRENT_CONTEXT_SCOPE_HPP */
