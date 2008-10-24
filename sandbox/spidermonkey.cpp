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

// g++ -pedantic -Wall -W -std=c++98 -g3 sandbox/spidermonkey.cpp -I/opt/local/include/ -L/opt/local/lib -ljs -DXP_UNIX

#include "js/jsapi.h"

#include <iostream>
#include <cstring>

static JSClass global_class = {
  "global", JSCLASS_GLOBAL_FLAGS,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
  JSCLASS_NO_OPTIONAL_MEMBERS
};

void reportError(JSContext *, char const *message, JSErrorReport *report) {
  std::cerr << (report->filename ? report->filename : "<no filename>")
            << '@'
            << (unsigned int) report->lineno
            << ": `"
            << message
            << "'\n";
}

int main() {
  JSRuntime *rt;
  JSContext *cx;
  JSObject  *global;

  rt = JS_NewRuntime(8L * 1024L * 1024L);
  if(!rt)
    return 1;

  cx = JS_NewContext(rt, 8192);
  if(!cx)
    return 1;
  JS_SetOptions(cx, JSOPTION_VAROBJFIX);
  //JS_SetVersion(cx, JSVERSION_LATEST);
  JS_SetErrorReporter(cx, reportError);

  global = JS_NewObject(cx, &global_class, NULL, NULL);
  if(!global)
    return 1;

  if(!JS_InitStandardClasses(cx, global))
    return 1;

  jsval rval;
  char const *source0 = "var hw = 'Hello World\\n';";
  
  JSBool ok = JS_EvaluateScript(cx, global, source0, std::strlen(source0),
                                __FILE__, 0, &rval);
  if(!ok)
    return 1;

  char const *source1 = "hw";
  ok = JS_EvaluateScript(cx, global, source1, std::strlen(source1),
                         __FILE__, 1, &rval);
  if(ok) {
    if(JSVAL_IS_STRING(rval)) {
      JSString *ret = JS_ValueToString(cx, rval);
      if(!ret)
        return 1;
      std::cout << JS_GetStringBytes(ret);
    }
    else {
      return 1;
    }
  }

  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);
  JS_ShutDown();
  return 0;
}
