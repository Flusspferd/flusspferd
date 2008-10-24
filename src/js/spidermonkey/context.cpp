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

#include "flusspferd/js/context.hpp"
#include "flusspferd/js/object.hpp"
#include "flusspferd/js/exception.hpp"
#include "flusspferd/js/spidermonkey/value.hpp"
#include "flusspferd/js/spidermonkey/object.hpp"
#include "flusspferd/js/spidermonkey/runtime.hpp"
#include "flusspferd/js/current_context_scope.hpp"

#include <js/jsapi.h>

#ifndef FLUSSPFERD_STACKCHUNKSIZE
#define FLUSSPFERD_STACKCHUNKSIZE 8192
#endif

namespace flusspferd { namespace js {
  namespace {
    static JSClass global_class = {
      "global", JSCLASS_GLOBAL_FLAGS,
      JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
      JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
      JSCLASS_NO_OPTIONAL_MEMBERS
    };
  }

  class context::impl {
  public:
    impl()
      : context(JS_NewContext(Impl::get_runtime(),
                              FLUSSPFERD_STACKCHUNKSIZE)),
        destroy(true)
    {
      if(!context)
        throw exception("Could not create Spidermonkey Context");

      JS_SetOptions(context, JSOPTION_VAROBJFIX);
      JS_SetOptions(context, JSOPTION_DONT_REPORT_UNCAUGHT);

      JSObject *global_ = JS_NewObject(context, &global_class, 0x0, 0x0);
      if(!global_)
        throw exception("Could not create Global Object");

      JS_SetGlobalObject(context, global_);

      if(!JS_InitStandardClasses(context, global_))
        throw exception("Could not initialize Global Object");
    }

    explicit impl(JSContext *context)
      : context(context), destroy(false)
    { }

    ~impl() {
      if(destroy)
        JS_DestroyContext(context);
    }

    JSContext *context;
    bool destroy;
  };

  struct context::detail {
    JSContext *c;
    detail(JSContext *ct) : c(ct) { }

    static JSContext *get(context &co) {
      return co.p->context;
    }
  };

  namespace Impl {
    JSContext *get_context(context &co) {
      return context::detail::get(co);
    }

    context wrap_context(JSContext *c) {
      return context(c);
    }
  }

  context::context()
  { }
  context::context(context::detail const &d)
    : p(new impl(d.c))
  { }
  context::~context() { }

  context context::create() {
    context c;
    c.p.reset(new impl);
    return c;
  }

  bool context::is_valid() const {
    return p;
  }

  object context::global() {
    return Impl::wrap_object(JS_GetGlobalObject(p->context));
  }

  value context::evaluate(char const *source, std::size_t n,
                          char const *file, unsigned int line)
  {
    current_context_scope scope(*this);
    jsval rval;
    JSBool ok = JS_EvaluateScript(p->context, JS_GetGlobalObject(p->context),
                                  source, n, file, line, &rval);
    if(!ok) {
      throw exception("Could not evaluate script");
    }
    return Impl::wrap_jsval(rval);
  }

  value context::evaluate(char const *source, char const *file,
                          unsigned int line)
  {
    return evaluate(source, std::strlen(source), file, line);
  }

  value context::evaluate(std::string const &source, char const *file,
                          unsigned int line)
  {
    return evaluate(source.data(), source.size(), file, line);
  }

  void context::gc() {
    JS_GC(p->context);
  }
}}
