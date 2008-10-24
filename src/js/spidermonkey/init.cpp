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

#include "flusspferd/init.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/context.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include <js/jsapi.h>

#ifndef FLUSSPFERD_MAX_BYTES
#define FLUSSPFERD_MAX_BYTES 8L * 1024L * 1024L // 8 MB TODO: too much?
#endif

using namespace flusspferd;

class init::impl {
public:
  // we use a single JS_Runtime for each process!
  impl()
    : runtime(JS_NewRuntime( FLUSSPFERD_MAX_BYTES ))
  {
    if(!runtime) {
      throw exception("Could not create Spidermonkey Runtime");
    }
  }
  ~impl() {
    JS_DestroyRuntime(runtime);
  }

  JSRuntime *runtime;
  context current_context;
};

struct init::detail {
  static JSRuntime *get(init &in) {
    return in.p->runtime;
  }
};

JSRuntime *Impl::get_runtime() {
  return init::detail::get(init::initialize());
}

init::init() : p(new impl) { }
init::~init() {
  JS_ShutDown();
}

context init::enter_current_context(context const &c) {
  context old = p->current_context;
  p->current_context = c;
  return old;
}

bool init::leave_current_context(context const &c) {
  if (c == p->current_context) {
    p->current_context = context();
    return true;
  } else {
    return !p->current_context.is_valid();
  }
}

context &init::get_current_context() {
  return p->current_context;
}

object flusspferd::global() {
  return get_current_context().global();
}
