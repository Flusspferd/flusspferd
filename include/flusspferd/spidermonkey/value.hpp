// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

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

#ifndef FLUSSPFERD_SPIDERMONKEY_VALUE_HPP
#define FLUSSPFERD_SPIDERMONKEY_VALUE_HPP

#include <js/jsapi.h>

namespace flusspferd {

class string;
class object;

#ifndef IN_DOXYGEN

namespace Impl {

class value_impl {
  jsval val;
  jsval *ref;

protected:
  jsval get() const { return *ref; }
  jsval *getp()     { return ref; }
  void set(jsval v) { *ref = v; }
  void setp(jsval *p) { ref = p; }
  void setval(jsval v) { val = v; }
  jsval *getvalp() { return &val; }

  value_impl(jsval v) : val(v), ref(&val) { }
  value_impl(jsval *v) : val(JSVAL_VOID), ref(v) { }
  value_impl() : val(JSVAL_VOID), ref(&val) { }

  friend jsval get_jsval(value_impl const &v);
  friend value_impl wrap_jsval(jsval v);
  friend jsval *get_jsvalp(value_impl &v);
  friend value_impl wrap_jsvalp(jsval *p);

  template<typename T>
  static value_impl from_integer(T const &num);
  static value_impl from_double(double num);
  static value_impl from_boolean(bool x);
  static value_impl from_string(string const &x);
  static value_impl from_object(object const &x);

public:
  value_impl(value_impl const &o) {
    if (o.ref == &o.val) {
      val = o.val;
      ref = &val;
    } else {
      val = JSVAL_VOID;
      ref = o.ref;
    }
  }

  value_impl &operator=(value_impl const &o) {
    *ref = *o.ref;
    return *this;
  }

  void *get_gcptr() {
    return getp();
  }
};

inline jsval get_jsval(value_impl const &v) {
  return v.get();
}

inline value_impl wrap_jsval(jsval v) {
  return value_impl(v);
}

inline jsval *get_jsvalp(value_impl &v) {
  return v.getp();
}

inline value_impl wrap_jsvalp(jsval *p) {
  return value_impl(p);
}

template<typename T>
value_impl value_impl::from_integer(T const &num) {
  if (INT_FITS_IN_JSVAL(num)) {
    return wrap_jsval(INT_TO_JSVAL(num));
  } else {
    //TODO: check range?
    return from_double(num);
  }
}

}

#endif

}

#endif /* FLUSSPFERD_SPIDERMONKEY_VALUE_HPP */
