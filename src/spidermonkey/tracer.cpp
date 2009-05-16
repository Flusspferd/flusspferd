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

#include "flusspferd/tracer.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/spidermonkey/value.hpp"
#include <js/jsapi.h>

using namespace flusspferd;

class tracer::impl {
public:
  impl(void *x)
#if JS_VERSION >= 180
  : trc((JSTracer*) x)
#else
  : cx(Impl::current_context()), x(x)
#endif
  {}

#if JS_VERSION >= 180
  JSTracer *trc;
#else
  JSContext *cx;
  void *x;
#endif
};

tracer::tracer(void *x) : p(new impl(x)) { }

tracer::~tracer() {}

void tracer::trace_gcptr(char const *name, void *gcthing) {
  if (!gcthing)
    return;

  jsval v = * (jsval *) gcthing;

#if JS_VERSION >= 180
  JS_CALL_VALUE_TRACER(p->trc, v, name);
#else
  if (!JSVAL_IS_GCTHING(v))
    return;
  JS_MarkGCThing(p->cx, JSVAL_TO_GCTHING(v), name, p->x);
#endif
}
