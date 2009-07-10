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

#include "flusspferd/exception.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/root.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/spidermonkey/value.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <js/jsapi.h>

using namespace flusspferd;

namespace {
  std::string exception_message(std::string const &what) {
    std::string ret = what;
    jsval v;
    JSContext *const cx = Impl::current_context();

    if (JS_GetPendingException(cx, &v)) {
      value val = Impl::wrap_jsval(v);
      ret += ": exception `" + val.to_std_string() + '\'';
      if (val.is_object()) {
        object o = val.to_object();
        if(o.has_property("fileName"))
          ret += " at " + o.get_property("fileName").to_std_string()
              +  ":" + o.get_property("lineNumber").to_std_string();
      }
      return ret;
    }

    return ret;
  }
}

class exception::impl {
public:
  impl() : empty(true) {}

  ~impl();

  context ctx;
  boost::scoped_ptr<root_value> exception_value;
  bool empty;
};

exception::exception(char const *what, std::string const &type)
  : std::runtime_error(exception_message(what))
{
  boost::shared_ptr<impl> p(new impl);

  p->exception_value.reset(new root_value);
  p->ctx = current_context();

  JSContext *ctx = Impl::get_context(p->ctx);

  value &v = *p->exception_value;

  if (JS_GetPendingException(ctx, Impl::get_jsvalp(v))) {
    p->empty = false;
    JS_ClearPendingException(ctx);
  } else {
    try {
      v = global().call(type, what);
    } catch (...) { }
  }

  this->p = p;
}

exception::exception(value const &val)
  : std::runtime_error(val.to_std_string()), p(new impl)
{
  p->exception_value.reset(new root_value(val));
  p->ctx = current_context();
}

exception::~exception() throw()
{
}

exception::impl::~impl() {
  if (exception_value) {
    current_context_scope scope(ctx);
    exception_value.reset();
  }
}

void exception::throw_js_INTERNAL() {
  JS_SetPendingException(
      Impl::current_context(),
      Impl::get_jsval(p->exception_value ? *p->exception_value : value()));
}

value exception::val() const {
  return p->exception_value ? *p->exception_value : value();
}

bool exception::empty() const {
  return p->empty;
}



js_quit::js_quit() {}

js_quit::~js_quit() {}
