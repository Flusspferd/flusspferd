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

#ifndef TEMPLAR_JS_ARGUMENTS_HPP
#define TEMPLAR_JS_ARGUMENTS_HPP

#include "implementation/arguments.hpp"
#include <vector>

namespace flusspferd { namespace js {
  class value;

  class arguments : public Impl::arguments_impl {
  public:
    arguments() {}
    arguments(Impl::arguments_impl const &a)
      : Impl::arguments_impl(a)
    { }
    arguments(std::vector<value> const &v);

    std::size_t size() const;
    value operator[](std::size_t i);

    void push_back(value const &v);
    value front();
    value back();

    class iterator : public Impl::arguments_impl::iterator_impl {
    public:
      iterator(Impl::arguments_impl::iterator_impl const &i)
        : Impl::arguments_impl::iterator_impl(i)
      { }

      iterator &operator++();
      iterator operator++(int) {
        iterator tmp(*this);
        ++*this;
        return tmp;
      }

      value operator*() const;
    };

    iterator begin();
    iterator end();
  };

  bool operator!=(arguments::iterator const &lhs,
                  arguments::iterator const &rhs);
  inline bool operator==(arguments::iterator const &lhs,
                         arguments::iterator const &rhs)
  {
    return !(lhs != rhs);
  }
}}

#endif /* TEMPLAR_JS_ARGUMENTS_HPP */
