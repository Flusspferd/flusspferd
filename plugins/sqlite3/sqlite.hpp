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
#ifndef GUARD_FLUSSPFERD_PLUGINS_SQLITE3_SQLITE_HPP_INCLUDED
#define GUARD_FLUSSPFERD_PLUGINS_SQLITE3_SQLITE_HPP_INCLUDED

#include "flusspferd.hpp"
#include <sqlite3.h>
#include "sqlite_cursor.hpp"

namespace sqlite3_plugin{

void raise_sqlite_error(sqlite3* db);

FLUSSPFERD_CLASS_DESCRIPTION(
    sqlite3,
    (full_name, "SQLite3")
    (constructor_name, "SQLite3")
    (methods,
        ("query", bind, query)
        ("exec", bind, exec)
        ("execMany", bind, execMany)
        ("close", bind, close)
        ("lastInsertID", bind, last_insert_id)
        ("begin", bind, begin)
        ("commit", bind, commit)
        ("rollback", bind, rollback))
    (constructor_properties,
        ("version", constant, SQLITE_VERSION_NUMBER)
        ("versionStr", constant, SQLITE_VERSION)))
{
public:
    sqlite3(flusspferd::object const &obj, 
            flusspferd::call_context &x);

    ~sqlite3();

public: // JS methods
    ::sqlite3 *db;

    void close();
    void query(flusspferd::call_context &x);
    void exec(flusspferd::call_context & x);
    void execMany(flusspferd::call_context & x);
    void last_insert_id(flusspferd::call_context &x);

    void begin();
    void commit();
    void rollback();

protected:
    int exec_internal( flusspferd::array arr );
    flusspferd::object compile(flusspferd::string sql, flusspferd::value bind);
    void ensure_opened();
};

}

#endif //GUARD_FLUSSPFERD_PLUGINS_SQLITE3_SQLITE_HPP_INCLUDED

