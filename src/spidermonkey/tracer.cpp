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

#include "flusspferd/tracer.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/implementation/init.hpp"
#include "flusspferd/implementation/value.hpp"
#include <js/jsapi.h>
#include <iostream>//FIXME

using namespace flusspferd;

class tracer::impl {
public:
  impl(JSContext *cx, void *x) : cx(cx), x(x) {}

  JSContext *cx;
  void *x;
};

tracer::tracer(void *x) : p(new impl(Impl::current_context(), x)) { }

tracer::~tracer() {}

void tracer::operator() (char const *name, void *gcthing) {
  std::cout << "TRACE " << name << " => " << gcthing << std::endl;
  if (!gcthing)
    return;
  jsval v = * (jsval *) gcthing;
  if (!JSVAL_IS_GCTHING(v))
    return;
  JS_MarkGCThing(p->cx, JSVAL_TO_GCTHING(v), name, p->x);
}
