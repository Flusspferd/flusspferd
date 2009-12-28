// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
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

#include "flusspferd/exception.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/root.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/spidermonkey/value.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include <boost/version.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/get_error_info.hpp>
#include <js/jsapi.h>

using namespace flusspferd;


struct exception::impl {
  impl(std::string const &what, std::string const &type);
  impl(value const &v);
  ~impl();

  context ctx;
  boost::scoped_ptr<root_value> exception_value;
  bool is_js_exception;
};

exception::impl::impl(value const &v)
  : ctx(current_context()),
    exception_value(new root_value(v)),
    is_js_exception(true)
{ }

exception::impl::impl(std::string const &what, std::string const &type)
  : ctx(current_context()),
    exception_value(new root_value),
    is_js_exception(true)
{
  JSContext *const cx = Impl::get_context(ctx);

  if (JS_GetPendingException(cx, Impl::get_jsvalp(*exception_value))) {
    is_js_exception = false;
    JS_ClearPendingException(cx);
  } else {
    try {
      *exception_value = global().call(type, what);
    } catch (...) { }
  }
}

exception::impl::~impl() {
  if (exception_value) {
    current_context_scope scope(ctx);
    exception_value.reset();
  }
}

namespace {
std::string exception_message(std::string what) {
  jsval v;
  JSContext *const cx = Impl::current_context();

  if (JS_GetPendingException(cx, &v)) {
    value val = Impl::wrap_jsval(v);
    what += ": exception `" + val.to_std_string() + '\'';
    if (val.is_object()) {
      object o = val.to_object();
      if(o.has_property("fileName"))
        what += " at " + o.get_property("fileName").to_std_string()
             +  ':' + o.get_property("lineNumber").to_std_string();
    }
  }

  return what;
}
}

exception::exception(char const *what, std::string const &type)
  : std::runtime_error(exception_message(what)),
    p(new impl(what, type))
{ }

exception::exception(std::string const &what, std::string const &type)
  : std::runtime_error(exception_message(what.c_str())),
    p(new impl(what, type))
{ }

exception::exception(value const &val)
  : std::runtime_error(val.to_std_string()),
    p(new impl(val))
{ }

exception::exception(boost::format const &fmt, std::string const &type)
  : std::runtime_error(exception_message(fmt.str().c_str())),
    p(new impl(fmt.str(), type))
{ }

exception::~exception() throw()
{ }

void exception::throw_js_INTERNAL() {
  JS_SetPendingException(
      Impl::current_context(),
      Impl::get_jsval(p->exception_value ? *p->exception_value : value()));
}

value exception::val() const {
  return p->exception_value ? *p->exception_value : value();
}

bool exception::is_js_exception() const {
  return p->is_js_exception;
}

char const *exception::what() const throw() {
#ifdef DEBUG
  char const * const *file;
  int const *line;
  char const * const *func;
  if( (file = boost::get_error_info<boost::throw_file>(*this)) &&
      (line = boost::get_error_info<boost::throw_line>(*this)) &&
      (func = boost::get_error_info<boost::throw_function>(*this)))
  {
    std::stringstream sstr;
    sstr << '(' << *func << ':' << *file << '@' << *line << ") " << std::runtime_error::what();
    return sstr.str().c_str();
  }
#endif

  return std::runtime_error::what();
}

js_quit::js_quit() {}

js_quit::~js_quit() {}
