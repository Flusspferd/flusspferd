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

#include "flusspferd/system.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/create/object.hpp"
#include "flusspferd/create/array.hpp"
#include "flusspferd/create/native_object.hpp"
#include "flusspferd/version.hpp"
#include "flusspferd/io/stream.hpp"
#include <boost/fusion/include/make_vector.hpp>
#include <iostream>
#include <ostream>


#if defined(__APPLE__)
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
#elif defined(WIN32)
#include <windows.h>
#else
extern char** environ;
# endif

using namespace flusspferd;
namespace fusion = boost::fusion;

// The class for sys.env
FLUSSPFERD_CLASS_DESCRIPTION(
  environment,
  (constructor_name, "Environment")
  (full_name, "system.Environment")
  (custom_enumerate, true)
  (methods,
      ("toString", bind, to_string)
  )
)
{
public:
  environment(object const &obj, call_context &);

  string to_string();

protected:
  boost::any enumerate_start(int &n);
  value enumerate_next(boost::any &iter);
  bool property_resolve(value const &id, unsigned access);
  void property_op(property_mode mode, value const &id, value &data);
};


void flusspferd::load_system_module(object &context) {
  object exports = context.get_property_object("exports");

  context.call("require", "io");

  exports.define_property(
    "stdout",
    create<io::stream>(fusion::make_vector(std::cout.rdbuf())),
    read_only_property | permanent_property);

  exports.define_property(
    "stderr",
    create<io::stream>(fusion::make_vector(std::cerr.rdbuf())),
    read_only_property | permanent_property);

  exports.define_property(
    "stdin",
    create<io::stream>(fusion::make_vector(std::cin.rdbuf())),
    read_only_property | permanent_property);


  load_class<environment>(create<object>());
  call_context x;

  exports.define_property(
    "env",
    create<environment>(fusion::vector1<call_context&>(x)),
    read_only_property | permanent_property
  );

  exports.define_property(
    "args",
    create<array>(),
    read_only_property | permanent_property);

  exports.define_property(
    "platform",
    value("flusspferd"),
    read_only_property | permanent_property);
}


environment::environment(object const &obj, call_context &)
  : base_type(obj)
{
}

bool environment::property_resolve(value const &id, unsigned)
{
  string name = id.to_string();
  if (name == "__iterator__")
    return false;

  char *val = getenv(name.c_str());
  if (!val)
    return false;

  define_property(name, string(val));
  return true;
}

boost::any environment::enumerate_start(int &n) {
  n = 0; // We dont know how many
#ifdef WIN32
  return boost::any(GetEnvironmentStrings());
#else
  return boost::any(environ);
#endif
}

value environment::enumerate_next(boost::any &iter) {
#ifdef WIN32
  // GetEnvironmentStrings returns "Var1=Value1\0Var2=Value2\0\0"
  char *env = boost::any_cast<char*>(iter);

  if (*env == 0) {
    FreeEnvironmentStrings(env);
    return value();
  }

  char* eq_c = strchr(env, '=');
  string s = string(env, eq_c - env);

  // Skip over the value, leaving env[0] at the \0 byte.
  while(*env) { ++env; }
  // Iter is now first byte of next var (or the second \0 of end)
  iter = ++env;
#else
  char **env = boost::any_cast<char**>(iter);

  if (*env == 0)
    return value();

  char* eq_c = strchr(*env, '=');
  string s = string(*env, eq_c - *env);

  iter = ++env;
#endif

  return s;
}

string environment::to_string() {
  return "[object sys.Environment]";
}

void environment::property_op(property_mode mode, value const &id, value &data) {
  switch (mode) {
  case property_get:
    // Not needed.
    break;
#ifdef WIN32
  default:
    SetEnvironmentVariable(
      id.to_std_string().c_str(),
      mode == property_delete ? NULL : data.to_std_string().c_str()
    );
#else
  case property_add:
  case property_set:
    setenv( id.to_std_string().c_str(), data.to_std_string().c_str(), 1 );
    break;
  case property_delete:
    unsetenv( id.to_std_string().c_str() );
    break;
#endif
  }
}
