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

#ifndef TEMPLAR_JS_NATIVE_FUNCTION_BASE_HPP
#define TEMPLAR_JS_NATIVE_FUNCTION_BASE_HPP

#include "init.hpp"
#include "function.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <string>

namespace templar { namespace js {

struct call_context;

class native_function_base : boost::noncopyable {
public:
  native_function_base(unsigned arity = 0);
  native_function_base(unsigned arity, std::string const &name);
  virtual ~native_function_base();

  void set_name(std::string const &name);
  std::string const &get_name() const;

  void set_arity(unsigned arity);
  unsigned get_arity() const;

protected:
  virtual void call(call_context &) = 0;

private:
  function create_function();

  friend class function;

private:
  class impl;
  boost::scoped_ptr<impl> p;

  friend class impl;
};

}}

#endif
