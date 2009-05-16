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

#ifndef FLUSSPFERD_SPIDERMONKEY_ARGUMETNS_HPP
#define FLUSSPFERD_SPIDERMONKEY_ARGUMETNS_HPP

#include <vector>
#include <js/jsapi.h>

namespace flusspferd {

class value;

#ifndef IN_DOXYGEN

namespace Impl {

class arguments_impl {
  std::vector<jsval> values; // values from the user are added here
  std::size_t n;
  jsval *argv;

public:
  arguments_impl(std::size_t n, jsval *argv) : n(n), argv(argv) { }
  arguments_impl(arguments_impl const &o);

protected:
  arguments_impl() : n(0), argv(0x0) {}
  arguments_impl(std::vector<value> const &o);

protected:
  jsval const *get() const { return argv; }
  jsval *get() { return argv; }
  std::size_t size() const { return n; }

  std::vector<jsval> &data() { return values; }
  std::vector<jsval> const &data() const { return values; }
  void reset_argv();

  bool is_userprovided() const {
    return values.size() == n; // TODO does this fix the problem?
  }

  arguments_impl &operator=(arguments_impl const &o);

  class iterator_impl {
    jsval *iter;
  public:
    iterator_impl(jsval *iter) : iter(iter) { }
    iterator_impl &operator++() {
      ++iter;
      return *this;
    }
    jsval *operator*() const { return iter; }
  };

  friend jsval *get_arguments(arguments_impl &);
};

inline jsval *get_arguments(arguments_impl &arg) {
  return arg.get();
}

}

#endif

}

#endif /* FLUSSPFERD_SPIDERMONKEY_ARGUMETNS_HPP */
