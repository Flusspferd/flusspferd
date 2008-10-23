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

#ifndef TEMPLAR_JS_NATIVE_OBJECT_BASE_HPP
#define TEMPLAR_JS_NATIVE_OBJECT_BASE_HPP

#include "object.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <memory>

namespace templar { namespace js {

struct call_context;
class tracer;

class native_object_base : public object, private boost::noncopyable {
protected:
  native_object_base();

  virtual void post_initialize();

  typedef void (native_object_base::*native_method_type)(call_context &);

  void add_native_method(std::string const &name, unsigned arity = 0);
  void add_native_method(std::string const &name, unsigned arity, native_method_type method);

  template<class T>
  void add_native_method(
    std::string const &name, unsigned arity, void (T::*method)(call_context&))
  {
    add_native_method(name, arity, native_method_type(method));
  }

  virtual void call_native_method(std::string const &name, call_context &);

  virtual void trace(tracer &);

public:
  virtual ~native_object_base() = 0;

  object get_object() {
    return *static_cast<object*>(this);
  }

  static native_object_base *get_native(object const &o);

private:
  void invalid_method(call_context &);

private:
  object create_object();

  friend class object;

public:
  class impl;
  boost::scoped_ptr<impl> p;

  friend class impl;
};

}}

#endif
