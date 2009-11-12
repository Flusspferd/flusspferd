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

#include "flusspferd/security.hpp"
#include "flusspferd/create/object.hpp"
#include "flusspferd/create/native_object.hpp"
#include "flusspferd/local_root_scope.hpp"

using namespace flusspferd;

security &security::create(object container) {
  security &obj = flusspferd::create<security>(
      param::_prototype = flusspferd::create<object>().prototype());
  root_object root_obj(obj);

  container.define_property("$security", obj);

  return obj;
}

security &security::get() {
  object scope = current_context().scope_chain();

  value v;
  while (!scope.is_null()) {
    v = scope.get_property("$security");

    if (!v.is_undefined_or_null())
      break;
    scope = scope.parent();
  }

  if (!v.is_object() || v.is_null())
    throw exception("Could not get security object");

  object obj = v.get_object();

  return flusspferd::get_native<security>(obj);
}

class security::impl {
public:
};

security::security(object const &obj)
  : base_type(obj), p(new impl)
{}

security::~security()
{}

bool security::check_path(std::string const &, unsigned) {
  // TODO
  return true;
}

bool security::check_url(std::string const &, unsigned) {
  // TODO
  return true;
}
