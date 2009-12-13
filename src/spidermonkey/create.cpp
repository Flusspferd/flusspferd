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

#include "flusspferd/create.hpp"
#include "flusspferd/create/object.hpp"
#include "flusspferd/create/array.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/create/native_object.hpp"
#include "flusspferd/create/native_function.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/native_function_base.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/spidermonkey/object.hpp"
#include "flusspferd/spidermonkey/init.hpp"
#include <js/jsapi.h>

using namespace flusspferd;

object flusspferd::detail::create_object(
  object const &proto, object const &parent)
{
  JSObject *o = JS_NewObject(
      Impl::current_context(),
      0,
      Impl::get_object(proto),
      Impl::get_object(parent));

  if (!o)
    throw exception("Could not create object");

  return Impl::wrap_object(o);
}

array flusspferd::detail::create_length_array(std::size_t length) {
  JSObject *o = JS_NewArrayObject(Impl::current_context(), length, 0);
  if (!o)
    throw exception("Could not create array");

  return object(Impl::wrap_object(o));
}

object flusspferd::detail::create_native_object(
  object const &proto, object const &parent)
{
  return native_object_base::do_create_object(proto, parent);
}

object flusspferd::detail::create_native_enumerable_object(
  object const &proto, object const &parent)
{
  return native_object_base::do_create_enumerable_object(proto, parent);
}

function flusspferd::detail::create_source_function(
    flusspferd::string const &name,
    std::vector<flusspferd::string> const &argnames,
    flusspferd::string const &body,
    flusspferd::string const &file,
    unsigned line)
{
  JSContext *cx = Impl::current_context();

  std::vector<char const *> argnames_c;
  argnames_c.reserve(argnames.size());

  for (std::vector<flusspferd::string>::const_iterator it = argnames.begin();
      it != argnames.end();
      ++it)
    argnames_c.push_back(it->c_str());

  JSFunction *fun =
      JS_CompileUCFunction(
        cx,
        0,
        name.c_str(),
        argnames_c.size(),
        &argnames_c[0],
        body.data(),
        body.length(),
        file.c_str(),
        line);

  if (!fun)
    throw exception("Could not compile function");

  return Impl::wrap_function(fun);
}
