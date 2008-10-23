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

#ifndef TEMPLAR_JS_EVALUATE_HPP
#define TEMPLAR_JS_EVALUATE_HPP

#include <templar/js/init.hpp>
#include <templar/js/context.hpp>

namespace templar { namespace js {
  inline value evaluate(char const *source, std::size_t n,
                        char const *file = 0x0, unsigned int line = 0)
  {
    return get_current_context().evaluate(source, n, file, line);
  }

  inline value evaluate(char const *source, char const *file = 0x0,
                        unsigned int line = 0)
  {
    return get_current_context().evaluate(source, file, line);
  }

  inline value evaluate(std::string const &source, char const *file = 0x0,
                        unsigned int line = 0)
  {
    return get_current_context().evaluate(source, file, line);
  }
}}

#endif /* TEMPLAR_JS_EVALUATE_HPP */
