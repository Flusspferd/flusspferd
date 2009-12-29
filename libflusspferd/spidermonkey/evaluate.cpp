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
#include "flusspferd/evaluate.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include "flusspferd/modules.hpp"

#include <js/jsapi.h>

#include <cstring>

using namespace flusspferd;

value flusspferd::evaluate(
  char const *source,
  std::size_t n,
  char const *file,
  unsigned int line)
{
  return evaluate_in_scope(source, n, file, line, global());
}

// Unicode version of the above
value flusspferd::evaluate(
  string const &source, char const *file, unsigned int line)
{
  return evaluate_in_scope(source, file, line, global());
}


value flusspferd::evaluate_in_scope(
  char const* source,
  std::size_t n,
  char const* file,
  unsigned int line,
  object const &scope)
{
  JSContext *cx = Impl::current_context();

  jsval rval;
  JSBool ok = JS_EvaluateScript(cx, Impl::get_object(scope),
                                source, n, file, line, &rval);
  if(!ok) {
    exception e("Could not evaluate script");
    if (!e.is_js_exception())
      throw e;
  }
  return Impl::wrap_jsval(rval);
}

// Unicode version of the above
value flusspferd::evaluate_in_scope(
  string const &source,
  char const* file,
  unsigned int line,
  object const &scope)
{
  JSContext *cx = Impl::current_context();

  jsval rval;
  JSBool ok = JS_EvaluateUCScript(cx, Impl::get_object(scope),
                                source.data(), source.length(), file, line,
                                &rval);
  if(!ok) {
    exception e("Could not evaluate script");
    if (!e.is_js_exception())
      throw e;
  }
  return Impl::wrap_jsval(rval);
}

value flusspferd::evaluate_in_scope(std::string const &source,
                                    char const* file, unsigned int line,
                                    object const &scope)
{
  return evaluate_in_scope(source.data(), source.size(), file, line, scope);
}

value flusspferd::evaluate_in_scope(char const *source,
                                    char const* file, unsigned int line,
                                    object const &scope)
{
  return evaluate_in_scope(source, std::strlen(source), file, line, scope);
}

value flusspferd::execute(char const *filename, object const &scope_) {
  JSContext *cx = Impl::current_context();

  root_string module_text(require::load_module_text(filename));

  root_object scope_r(scope_);

  JSObject *scope = Impl::get_object(scope_);

  if (!scope)
    scope = Impl::get_object(flusspferd::global());

  int oldopts = JS_GetOptions(cx);
  JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
  JSScript *script = JS_CompileUCScript(
    cx, scope,
    module_text.data(), module_text.length(),
    filename, 1ul
  );

  if (!script) {
    exception e("Could not compile script");
    JS_SetOptions(cx, oldopts);
    throw e;
  }

  JS_SetOptions(cx, oldopts);

  // JS_NewScriptObject is needed because otherwise the Garbage Collector
  // may go amok!
  root_object script_o(Impl::wrap_object(JS_NewScriptObject(cx, script)));

  root_value result;

  JSBool ok = JS_ExecuteScript(cx, scope, script, Impl::get_jsvalp(result));

  if (!ok)
    throw exception("Script execution failed");

  JS_MaybeGC(cx);

  return result;
}

value flusspferd::evaluate(
  char const *source, char const *file, unsigned int line)
{
  return evaluate(source, std::strlen(source), file, line);
}

value flusspferd::evaluate(
  std::string const &source, char const *file, unsigned int line)
{
  return evaluate(source.data(), source.size(), file, line);
}

bool flusspferd::is_compilable(char const *source, std::size_t length,
                                object const &scope_)
{
  assert(source);
  JSContext *cx = Impl::current_context();

  JSObject *scope = Impl::get_object(scope_);
  if (!scope) {
    scope = Impl::get_object(flusspferd::global());
  }

  return JS_BufferIsCompilableUnit(cx, scope, source, length);
}

bool flusspferd::is_compilable(char const *source, object const &scope) {
  return is_compilable(source, std::strlen(source), scope);
}

bool flusspferd::is_compilable(std::string const &source, object const &scope) {
  return is_compilable(source.data(), source.size(), scope);
}
