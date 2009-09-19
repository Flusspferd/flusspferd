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

#include "flusspferd.hpp"
#include <new>
#include <sstream>

#include <sqlite3.h>

using namespace flusspferd;

// Put everything in an anon-namespace so typeid wont clash ever.
namespace {

void raise_sqlite_error(sqlite3* db);

///////////////////////////
// Classes
///////////////////////////

FLUSSPFERD_CLASS_DESCRIPTION(
  sqlite3,
  (full_name, "SQLite3")
  (constructor_name, "SQLite3")
  (methods,
    ("cursor", bind, cursor)
    ("close", bind, close))
  (constructor_properties,
    ("version", constant, SQLITE_VERSION_NUMBER)
    ("versionStr", constant, SQLITE_VERSION)))
{
public:
  sqlite3(object const &obj, call_context &x);
  ~sqlite3();

public: // JS methods
  ::sqlite3 *db;

  void close();
  void cursor(call_context &x);
};

FLUSSPFERD_CLASS_DESCRIPTION(
  sqlite3_cursor,
  (constructible, false)
  (full_name, "SQLite3.Cursor")
  (constructor_name, "Cursor")
  (methods,
    ("close", bind, close)
    ("reset", bind, reset)
    ("next", bind, next)
    ("bind", bind, bind)))
{
public:
  sqlite3_cursor(object const &obj, sqlite3_stmt *sth);
  ~sqlite3_cursor();

private:
  sqlite3_stmt *sth;
  
  enum  {
    CursorState_Init = 0,
    CursorState_InProgress = 1,
    CursorState_Finished = 2,
    CursorState_Errored = 3
  } state;

  // Methods that help wiht binding
  void bind_array(array &a, size_t num_binds);
  void bind_dict(object &o, size_t num_binds);
  void do_bind_param(int n, value v);

public: // JS methods
  void close();
  void reset();
  object next();
  void bind(call_context &x);
};

FLUSSPFERD_LOADER_SIMPLE(exports) {
  object ctor = load_class<sqlite3>(exports);
  load_class<sqlite3_cursor>(ctor);
}

///////////////////////////
sqlite3::sqlite3(object const &obj, call_context &x)
  : base_type(obj),
    db(NULL)
{
  if (x.arg.size() == 0)
    throw exception ("SQLite3 requires more than 0 arguments");

  string dsn = x.arg[0];

  // TODO: pull arguments from 2nd/options argument
  if (sqlite3_open(dsn.c_str(), &db) != SQLITE_OK) {
    if (db)
      raise_sqlite_error(db);
    else
      throw std::bad_alloc(); // out of memory. better way to signal this?
  }
}
///////////////////////////
sqlite3::~sqlite3()
{
  close();
}

///////////////////////////
void sqlite3::close()
{
  if (db) {
    sqlite3_close(db);
    db = NULL;
  }
}

///////////////////////////
void sqlite3::cursor(call_context &x) {
  local_root_scope scope;
  if (!db)
    throw exception("SQLite3.cursor called on closed dbh");

  if (x.arg.size() < 1)
    throw exception ("cursor requires more than 0 arguments");

  string sql = x.arg[0].to_string();
  size_t n_bytes = sql.length() * 2;
  sqlite3_stmt *sth;
  char16_t *tail; // uncompiled part of the sql (when multiple stmts)
  if (sqlite3_prepare16_v2(db, sql.data(), n_bytes, &sth, (const void**)&tail) != SQLITE_OK)
  {
    raise_sqlite_error(db);
  }
  object cursor = create_native_object<sqlite3_cursor>(object(), sth);

  // TODO: remove tail and set it seperately
  cursor.define_property("sql", sql);

  x.result = cursor;
}

///////////////////////////
// 'Private' constructor that is called from sqlite3::cursor
sqlite3_cursor::sqlite3_cursor(object const &obj, sqlite3_stmt *_sth)
  : base_type(obj),
    sth(_sth),
    state(CursorState_Init)
{
}


///////////////////////////
sqlite3_cursor::~sqlite3_cursor()
{
  close();
}

///////////////////////////
void sqlite3_cursor::close() {
  if (sth) {
    sqlite3_finalize(sth);
    sth = NULL;
  }
}

///////////////////////////
void sqlite3_cursor::reset() {
  sqlite3_reset(sth);
  state = CursorState_Init;
}

///////////////////////////
object sqlite3_cursor::next() {
  local_root_scope scope;

  if (!sth)
    throw exception("SQLite3.Cursor.next called on closed cursor");

  switch (state) {
    case CursorState_Finished:
      // We've seen the last row, remember it and return the EOF indicator
      return object();
    case CursorState_Errored:
      throw exception("SQLite3.Cursor: This cursor has seen an error and "
                      "needs to be reset");
    default:
      break;    
  }

  int code = sqlite3_step(sth);

  if (code == SQLITE_DONE) {
    state = CursorState_Finished;
    return object();
  } else if (code != SQLITE_ROW) {
    if (sqlite3_errcode( sqlite3_db_handle(sth) ) != SQLITE_OK) {
      state = CursorState_Errored;
      raise_sqlite_error( sqlite3_db_handle(sth) );
    } else {
      throw exception("SQLite3.Cursor: database reported misuse error. "
                      "Please try again");
    }
  }

  state = CursorState_InProgress;

  // Build up the row object.
  array row = create_array();
  int cols = sqlite3_column_count(sth);

  for (int i=0; i < cols; i++)
  {
    int type = sqlite3_column_type(sth, i);
    value col;
    
    switch (type) {
      case SQLITE_INTEGER:
        col = value(sqlite3_column_int(sth, i));
        break;
      case SQLITE_FLOAT:
        col = value(sqlite3_column_double(sth, i));
        break;
      case SQLITE_NULL:
        col = object();
        break;
      //case SQLITE_BLOB:
        // TODO: Support binary data!
        //break;
      case SQLITE_TEXT:
        {
          char16_t *text  = (char16_t*)sqlite3_column_text16(sth, i);

          if (!text)
            throw std::bad_alloc();

          // Its actually num of *bytes* not chars
          size_t length = sqlite3_column_bytes16(sth, i) / 2;
          col = string(text, length);
        }
        break;
      default:
        {
          std::stringstream ss;
          ss << "SQLite3.Cursor.next: Unknown column type " << type;
          throw exception(ss.str().c_str());
        }
        break;
    }
    row.set_element(i, col);
  }
  return row;
}

///////////////////////////
// JS method
void sqlite3_cursor::bind(call_context &x) {
  local_root_scope scope;

  size_t num_binds = sqlite3_bind_parameter_count(sth);

  if (!num_binds)
    return;

  if (x.arg[0].is_object()) {
    object o = x.arg[0].get_object();

    if (o.is_array()) {
      array a = o;
      bind_array(a, num_binds);
    }
    else
      bind_dict(o, num_binds);
  } else if (num_binds == 1) {
    // Dont document that we accept a single param
    do_bind_param(1, x.arg[0]);
    
  } else {
    throw exception("SQLite3.Cursor.bind requires an array or an object as its only argument");
  }
}

///////////////////////////
// bind numbered params
void sqlite3_cursor::bind_array(array &a, size_t num_binds) {

  // Pull bind param 1 (the first) from array index 0 (also the first)
  for (size_t n = 1; n <= num_binds; n++) {
    value bind = a.get_element(n-1);

    if (!bind.is_undefined())
      do_bind_param(n, bind);

  }
}

///////////////////////////
// bind named or numbered params
void sqlite3_cursor::bind_dict(object &o, size_t num_binds) {

  for (size_t n = 1; n <= num_binds; n++) {
    const char* name = sqlite3_bind_parameter_name(sth, n);

    value bind;

    if (!name) {
      // Possibly a '?' unnnamed param
      // TODO: This will break when n is > 2^31.
      bind = o.get_property( value(n) );
    } else {
      // Named param
      bind = o.get_property(name);
    }

    if (!bind.is_undefined())
      do_bind_param(n, bind);

  }

}

///////////////////////////
// Bind the actual para
void sqlite3_cursor::do_bind_param(int n, value v) {
  int ok;
  if (v.is_int()) {
      ok = sqlite3_bind_int(sth, n, v.get_int());
  } else if (v.is_double()) {
    ok = sqlite3_bind_double(sth, n, v.get_double());
  } else if (v.is_null()) {
    ok = sqlite3_bind_null(sth, n);
  } else {
    // Default, stringify the object
    string bind = v.to_string();
    ok = sqlite3_bind_text16(sth, n, bind.data(), bind.length()*2, SQLITE_TRANSIENT);
  }

  if (ok != SQLITE_OK)
    raise_sqlite_error( sqlite3_db_handle(sth) ); 
}

///////////////////////////
// Helper function
void raise_sqlite_error(::sqlite3* db)
{
  std::string s = "SQLite3 Error: ";
  s += sqlite3_errmsg(db);
  throw exception(s.c_str());
}

}

