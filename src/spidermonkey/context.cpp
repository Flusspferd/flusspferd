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

#include "flusspferd/context.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include "flusspferd/spidermonkey/value.hpp"
#include "flusspferd/spidermonkey/object.hpp"
#include "flusspferd/spidermonkey/runtime.hpp"
#include "flusspferd/current_context_scope.hpp"
#include <boost/unordered_map.hpp>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <js/jsapi.h>

#ifndef FLUSSPFERD_STACKCHUNKSIZE
#define FLUSSPFERD_STACKCHUNKSIZE 8192
#endif

using namespace flusspferd;

namespace {
  static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };
}

struct context::context_private {
  typedef boost::shared_ptr<root_object> root_object_ptr;
  boost::unordered_map<std::string, root_object_ptr> prototypes;
  boost::unordered_map<std::string, root_object_ptr> constructors;
};

/// impl provides the hidden implementation part
class context::impl {
public:
  impl()
    : context(JS_NewContext(Impl::get_runtime(),
                            FLUSSPFERD_STACKCHUNKSIZE)),
      destroy(true)
  {
    if(!context)
      throw exception("Could not create Spidermonkey Context");

    uint32 options = JS_GetOptions(context);

    options |= JSOPTION_VAROBJFIX;
    options |= JSOPTION_STRICT;
    options |= JSOPTION_DONT_REPORT_UNCAUGHT;
    options &= ~JSOPTION_XML;

    JS_SetVersion(context, JSVersion(JS_VERSION));
    JS_SetOptions(context, options);

#ifdef JS_THREADSAFE
    JS_BeginRequest(context);
#endif

    JS_SetErrorReporter(context, spidermonkey_error_reporter);

    JSObject *global_ = JS_NewObject(context, &global_class, 0x0, 0x0);
    if(!global_)
      throw exception("Could not create Global Object");

    // Set it so that it doesn't get GCd (play it safe)
    JS_SetGlobalObject(context, global_);

    if(!JS_InitStandardClasses(context, global_))
      throw exception("Could not initialize Global Object");

    JS_DeleteProperty(context, global_, "XML");

    // Set the global object as an empty object which has its proto as the
    // 'true' global object with the std classes on it
    JSObject *pub_global = JS_NewObject(context, &global_class, global_, NULL);
    if(!global_)
      throw exception("Could not create Global Shim Object");

    JS_SetGlobalObject(context, pub_global);

    JS_SetContextPrivate(context, static_cast<void*>(new context_private));
  }

  explicit impl(JSContext *context)
    : context(context), destroy(false)
  { }

  ~impl() {
    if (destroy) {
      {
        current_context_scope scope(Impl::wrap_context(context));
        delete get_private();
        JS_DestroyContext(context);
      }
    }
  }

  bool is_valid() {
    return context;
  }

  context_private *get_private() {
    return static_cast<context_private*>(JS_GetContextPrivate(context));
  }

  static void spidermonkey_error_reporter(JSContext *cx, char const *message, JSErrorReport *report) {

    if (!report || JSREPORT_IS_EXCEPTION(report->flags)) {
      return;
    }

    std::cerr << (JSREPORT_IS_STRICT(report->flags) ? "Strict warning: " : "Warning: ")
              << message;

    if (report->filename) {
      std::cerr << " at " << report->filename;

      if (report->lineno) {
        std::cerr << ":" << report->lineno;
      }
    }
    std::cerr << std::endl;

  }

  JSContext *context;
  bool destroy;
};

/// detail is used for copyconstructing/initialisation purpose
struct context::detail {
  JSContext *c;
  detail(JSContext *ct) : c(ct) { }

  static JSContext *get(context &co) {
    return co.p->context;
  }
};

JSContext *Impl::get_context(context &co) {
  return context::detail::get(co);
}

context Impl::wrap_context(JSContext *c) {
  return context(c);
}

context::context()
{ }
context::context(context::detail const &d)
  : p(new impl(d.c))
{
  if (!p->is_valid())
    p.reset();
}
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
  JSObject *o = JS_GetGlobalObject(p->context);
  if (!o) {
    throw exception("No global object");
  }
  return Impl::wrap_object(o);
}

object context::scope_chain() {
  JSObject *o = JS_GetScopeChain(p->context);
  if (!o) {
    // Weird! In Spidermonkey 1.7, sometimes it seems like JS_GetScopeChain
    // returns NULL without setting an error.
    // In that case, we simply return the global object. It caused a problem.
    if (JS_IsExceptionPending(p->context))
      throw exception("No scope chain");
    else
      return this->global();
  }
  return Impl::wrap_object(o);
}

void context::add_prototype(std::string const &name, object const &proto) {
  p->get_private()->prototypes[name] =
    context_private::root_object_ptr(new root_object(proto));
}

object context::prototype(std::string const &name) const {
  context_private::root_object_ptr ptr = p->get_private()->prototypes[name];
  return ptr ? *ptr : object();
}

void context::add_constructor(std::string const &name, object const &ctor) {
  p->get_private()->constructors[name] =
    context_private::root_object_ptr(new root_object(ctor));
}

object context::constructor(std::string const &name) const {
  context_private::root_object_ptr ptr = p->get_private()->constructors[name];
  return ptr ? *ptr : object();
}

void context::gc() {
  JS_GC(p->context);
}

void context::set_thread() {
#ifdef JS_THREADSAFE
  assert(JS_SetContextThread(p->context) == 0);
#endif
}

void context::clear_thread() {
#ifdef JS_THREADSAFE
  //assert(
  JS_ClearContextThread(p->context);
  // == js_GetCurrentThread(p->context->runtime)->id);
#endif
}

bool context::set_strict(bool strict) {
  uint32 options = JS_GetOptions(p->context);

  bool old = (options & JSOPTION_STRICT) == JSOPTION_STRICT;

  // No change.
  if (old == strict)
    return old;

  if (strict) {
    options |= JSOPTION_STRICT;
  }
  else {
    options &= ~JSOPTION_STRICT;
  }
  JS_SetOptions(p->context, options);

  return old;
}
