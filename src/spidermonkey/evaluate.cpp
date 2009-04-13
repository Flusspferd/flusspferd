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
#include "flusspferd/evaluate.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/implementation/init.hpp"

using namespace flusspferd;

value flusspferd::evaluate(
  char const *source,
  std::size_t n,
  char const *file,
  unsigned int line)
{
  return evaluate_in_scope(source, n, file, line, global());
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
    if (!e.empty())
      throw e;
  }
  return Impl::wrap_jsval(rval);
}

value flusspferd::execute(char const *filename, object const &scope_) {
  JSContext *cx = Impl::current_context();

  local_root_scope root_scope;

  FILE *file = fopen(filename, "r");
  if (!file) {
    throw exception((std::string("Could not open '") + filename + "'").c_str());
  }
 
  /*
   * It's not interactive - just execute it.
   *
   * Support the UNIX #! shell hack; gobble the first line if it starts
   * with '#'. TODO - this isn't quite compatible with sharp variables,
   * as a legal js program (using sharp variables) might start with '#'.
   * But that would require multi-character lookahead.
   */
  int ch = fgetc(file);
  if (ch == '#') {
      while((ch = fgetc(file)) != EOF) {
          if (ch == '\n' || ch == '\r')
              break;
      }
  }
  ungetc(ch, file);

  JSObject *scope = Impl::get_object(scope_);

  if (!scope)
    scope = Impl::get_object(flusspferd::global());
 
  int oldopts = JS_GetOptions(cx);
  JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO );
  JSScript *script = JS_CompileFileHandle(cx, scope, filename, file);

  if (!script) {
    exception e("Could not compile script");
    JS_SetOptions(cx, oldopts);
    throw e;
  }

  JS_SetOptions(cx, oldopts);

  value result;
 
  JSBool ok = JS_ExecuteScript(cx, scope, script, Impl::get_jsvalp(result));
 
  if (!ok) {
    exception e("Script execution failed");
    if (!e.empty())
      throw e;
  }

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


