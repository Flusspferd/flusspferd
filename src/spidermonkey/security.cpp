// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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
#include "flusspferd/create.hpp"
#include "flusspferd/local_root_scope.hpp"

using namespace flusspferd;

security &security::create(object container) {
  local_root_scope scope;

  security &obj = create_native_object<security>(
      create_object().get_prototype());

  container.define_property("$security", obj);

  return obj;
}

security &security::get() {
  object scope = get_current_context().scope_chain();

  value v;
  while (scope.is_valid()) {
    v = scope.get_property("$security");

    if (!v.is_undefined_or_null())
      break;
    scope = scope.get_parent();
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
  : native_object_base(obj), p(new impl)
{}

security::~security()
{}

bool security::check_path(std::string const &pathname, unsigned mode) {
  // TODO
  return true;
}

bool security::check_url(std::string const &url, unsigned mode) {
  // TODO
  return true;
}
