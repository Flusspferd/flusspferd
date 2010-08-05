// -*- mode:c++;coding:utf-8; -*- vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2010 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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
#ifndef FLUSSPFERD_SPIDERMONKEY_JSID_HPP
#define FLUSSPFERD_SPIDERMONKEY_JSID_HPP

#include "../exception.hpp"
#include "value.hpp"
#include "init.hpp"

/*
Jägermonkey changes the representation of jsval and jsid. 

http://blog.mozilla.com/rob-sayre/2010/08/02/mozillas-new-javascript-value-representation/
http://groups.google.com/group/mozilla.dev.tech.js-engine/browse_thread/thread/7c463ee33f32cfc0#

Therefore jsid can't be treated as a jsval. To wrap jsids into jsvals we need to convert them first.
 */

namespace flusspferd { namespace Impl {
inline value_impl wrap_jsid(jsid id) {
  jsval v;
  if(!JS_IdToValue(Impl::current_context(), id, &v)) {
    throw exception("could not convert id to value");
  }
  return wrap_jsval(v);
}

inline jsid get_jsid(value_impl const &v) {
  jsval jv = v.get();
  jsid id;
  if(!JS_ValueToId(Impl::current_context(), jv, &id)) {
    throw exception("could not convert value to id");
  }
  return id;
}
}}

#endif
