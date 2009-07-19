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

#include "flusspferd/system.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/io/stream.hpp"
#include <iostream>
#include <ostream>


#if defined(__APPLE__)
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
#elif defined(XP_WIN)
extern char** _environ;
#  define environ _environ
#else
extern char** environ;
# endif

using namespace flusspferd;

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
};


void flusspferd::load_system_module(object &context) {
  object exports = context.get_property_object("exports");

  context.call("require", "io");

  exports.define_property(
    "stdout",
    create_native_object<io::stream>(object(), std::cout.rdbuf()),
    read_only_property | permanent_property);

  exports.define_property(
    "stderr",
    create_native_object<io::stream>(object(), std::cerr.rdbuf()),
    read_only_property | permanent_property);

  exports.define_property(
    "stdin",
    create_native_object<io::stream>(object(), std::cin.rdbuf()),
    read_only_property | permanent_property);


  load_class<environment>(create_object());
  call_context x;

  exports.define_property(
    "env",
    create_native_object<environment>(object(), boost::ref(x)),
    read_only_property | permanent_property
  );

  exports.define_property(
    "args",
    create_array(),
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
  return boost::any(environ);
}

value environment::enumerate_next(boost::any &iter) {
  char **env = boost::any_cast<char**>(iter);

  if (*env == 0)
    return value();
  
  char* eq_c = strchr(*env, '=');
  string s = string(*env, eq_c - *env);

  iter = ++env;

  return s;
}

string environment::to_string() {
  return "[object sys.Environment]";
}
