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

#ifndef FLUSSPFERD_SPIDERMONKEY_OBJECT_HPP
#define FLUSSPFERD_SPIDERMONKEY_OBJECT_HPP

#include <js/jspubtd.h>
#include <boost/optional.hpp>

namespace flusspferd {

class context;
class object_template;
class object;

namespace Impl {

class object_impl {
  JSObject *obj;

protected:
  JSObject *get() { return obj; }
  JSObject *get_const() const { return obj; }
  void set(JSObject *o) { obj = o; }

  object_impl(JSObject *o) : obj(o) { }

  friend JSObject *get_object(object_impl const &o);
  friend object_impl wrap_object(JSObject *o);

public:
  void *get_gcptr() {
    return &obj;
  }
};

inline JSObject *get_object(object_impl const &o) {
  return o.get_const();
}
  
inline object_impl wrap_object(JSObject *o) {
  return object_impl(o);
}

inline bool operator==(object_impl const &a, object_impl const &b) {
  return get_object(a) == get_object(b);
}

inline bool operator!=(object_impl const &a, object_impl const &b) {
  return !(a == b);
}

}}

#endif /* FLUSSPFERD_SPIDERMONKEY_OBJECT_HPP */
