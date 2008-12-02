// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Ash Berlin

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

#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/tracer.hpp"

#include "sqlite3.h"

using namespace flusspferd;

///////////////////////////
class sqlite3 : public native_object_base {
public:
  struct class_info : public flusspferd::class_info {
    typedef boost::mpl::bool_<true> constructible;
    static char const* constructor_name() { return "SQLite3"; }
    static void augment_constructor(object &ctor);
  };

  sqlite3(object const &obj, call_context &x);
  ~sqlite3();

protected:
  //void trace(tracer &);

private: // JS methods
  object cursor(string sql);
};

///////////////////////////
// import hook
extern "C" value flusspferd_load(object container)
{
  return load_class<sqlite3>(container);
}

///////////////////////////
// Set version properties on constructor object
void sqlite3::class_info::augment_constructor(object &ctor)
{
  // Set static properties on the constructor
  ctor.define_property("version", SQLITE_VERSION_NUMBER, object::read_only_property | object::permanent_property);
  ctor.define_property("versionStr", string(SQLITE_VERSION), object::read_only_property | object::permanent_property);
}

///////////////////////////
sqlite3::sqlite3(object const &obj, call_context &)
  : native_object_base(obj)
{
}

///////////////////////////
sqlite3::~sqlite3()
{
}


