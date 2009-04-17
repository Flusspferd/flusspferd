// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld, Ash Berlin

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

#include "flusspferd/io/file.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/local_root_scope.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/string_io.hpp"
#include "flusspferd/create.hpp"
#include <boost/scoped_array.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>

#define creat _creat
#endif

using namespace flusspferd;
using namespace flusspferd::io;

class file::impl {
public:
  std::fstream stream;

  static void create(char const *name, boost::optional<int> mode);
  static bool exists(char const *name);
};

file::file(object const &obj, call_context &x)
  : stream(obj, 0), p(new impl)
{
  set_streambuf(p->stream.rdbuf());
  if (!x.arg.empty()) {
    string name = x.arg[0].to_string();
    open(name.c_str());
  }
}

file::~file()
{}

void file::class_info::augment_constructor(object constructor) {
  create_native_function(constructor, "create", &impl::create);
  create_native_function(constructor, "exists", &impl::exists);
}

object file::class_info::create_prototype() {
  local_root_scope scope;

  object proto = create_object(flusspferd::prototype<stream>());

  create_native_method(proto, "open", &file::open);
  create_native_method(proto, "close", &file::close);

  return proto;
}

void file::open(char const *name) {
  security &sec = security::get();

  if (!sec.check_path(name, security::READ_WRITE))
    throw exception("Could not open file (security)");

  p->stream.open(name);

  define_property("fileName", string(name), 
                  permanent_property | read_only_property );

  if (!p->stream)
    // TODO: Include some sort of system error message here
    throw exception("Could not open file");
}

void file::close() {
  p->stream.close();
  delete_property("fileName");
}

void file::impl::create(char const *name, boost::optional<int> mode) {
  security &sec = security::get();

  if (!sec.check_path(name, security::CREATE))
    throw exception("Could not create file (security)");

  if (creat(name, mode.get_value_or(0666)) < 0)
    throw exception("Could not create file");
}

bool file::impl::exists(char const *name) {
  security &sec = security::get();

  if (!sec.check_path(name, security::ACCESS))
    throw exception("Could not check whether file exists (security)");

  return boost::filesystem::exists(name);
}

