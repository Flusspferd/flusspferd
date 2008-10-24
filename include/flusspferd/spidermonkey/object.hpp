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

namespace flusspferd { namespace js {
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

    class property_iterator_impl {
      JSObject *iter;
      jsid id;
    public:
      property_iterator_impl() : iter(0x0) { }
      property_iterator_impl(JSObject *i) : iter(i) { ++*this; }

      jsid get_id() const { return id; }
      JSObject *get() { return iter; }
      JSObject *get_const() const { return iter; }
      property_iterator_impl &operator++();
    };

    friend JSObject *get_object(object_impl &o);
    friend object_impl wrap_object(JSObject *o);
  };

  inline JSObject *get_object(object_impl &o) {
    return o.get();
  }
  
  inline object_impl wrap_object(JSObject *o) {
    return object_impl(o);
  }
}}}

#endif /* FLUSSPFERD_SPIDERMONKEY_OBJECT_HPP */
