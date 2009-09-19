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
#ifndef GUARD_FLUSSPFERD_PLUGINS_SQLITE3_SQLITE_CURSOR_HPP_INCLUDED
#define GUARD_FLUSSPFERD_PLUGINS_SQLITE3_SQLITE_CURSOR_HPP_INCLUDED

#include "flusspferd.hpp"
#include <sqlite3.h>

namespace sqlite3_plugin {

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
    sqlite3_cursor(flusspferd::object const &obj, 
                   sqlite3_stmt *sth);
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
    void bind_array(flusspferd::array &a, size_t num_binds);
    void bind_dict(flusspferd::object &o, size_t num_binds);
    void do_bind_param(int n, flusspferd::value v);
    void raise_sqlite_error();
public: // JS methods
    void close();
    void reset();
    object next();
    void bind(flusspferd::call_context &x);    
};

}

#endif //GUARD_FLUSSPFERD_PLUGINS_SQLITE3_SQLITE_CURSOR_HPP_INCLUDED

