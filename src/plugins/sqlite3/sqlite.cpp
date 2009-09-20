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

#include "sqlite.hpp"
#include <boost/lexical_cast.hpp>

namespace sqlite3_plugin {

using namespace flusspferd;

void raise_sqlite_error(::sqlite3* db)
{
    std::string s = "SQLite3 Error: ";
    s += sqlite3_errmsg(db);
    throw exception(s.c_str());
}

sqlite3::sqlite3(object const &obj, call_context &x)
: base_type(obj)
, db(0)
{
    if (x.arg.size() == 0) {
        throw exception ("SQLite3 requires more than 0 arguments");
    }
    string dsn = x.arg[0];

    // TODO: pull arguments from 2nd/options argument
    if (sqlite3_open(dsn.c_str(), &db) != SQLITE_OK) {
        if (db) {
            raise_sqlite_error(db);
        }
        else {
            throw std::bad_alloc(); // out of memory. better way to signal this?
        }
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
    ensure_opened();

    if (x.arg.size() < 1) {
        throw exception ("cursor requires more than 0 arguments");
    }

    compile_result_t result = compile( x.arg[0].to_string() );
        
    object cursor = create_native_object<sqlite3_cursor>(
                        object(), 
                        boost::get<0>(result)
                    );

    cursor.define_property("sql", boost::get<1>(result));
    cursor.define_property("tail", boost::get<2>(result));        
    
    x.result = cursor;
}

///////////////////////////
void sqlite3::exec(call_context & x) {
    local_root_scope scope;

    if (x.arg.size() == 1) {
        throw exception ("SQLite3.exec() requires 1 argument");
    }

    compile_result_t result;
    string sql = x.arg[0].to_string();
    size_t count = 0; 
    do { 
        result = compile( sql );
    
        if ( sqlite3_step( boost::get<0>( result ) ) != SQLITE_DONE ) {
            raise_sqlite_error(db); 
        }    
        ++count;
        sql = boost::get<2>( result );
    } while ( !boost::get<2>( result ).empty() );

    x.result = count;
}

///////////////////////////
sqlite3::compile_result_t sqlite3::compile(flusspferd::string sql_in) {
    local_root_scope scope;

    size_t n_bytes = sql_in.length() * 2;
    sqlite3_stmt * sth = 0;
    char16_t * tail = 0; // uncompiled part of the sql (when multiple stmts)
    
    if (sqlite3_prepare16_v2(db, sql_in.data(), n_bytes, &sth, (const void**)&tail) != SQLITE_OK)
    {
        raise_sqlite_error(db);
    }

    string tail_str;
    if (tail) {
        tail_str = string(tail);
    }
    
    string sql = sql_in.substr( 0, sql_in.size() - tail_str.size() );
    
    return boost::make_tuple( sth, sql, tail_str );
}

///////////////////////////
void sqlite3::last_insert_id(call_context & x) {
    local_root_scope scope;
    ensure_opened();
        
    x.result = boost::lexical_cast< std::string >( sqlite3_last_insert_rowid( db ) );
}


///////////////////////////
void sqlite3::ensure_opened() {
    if (!db) {
        throw exception("SQLite3 method called on a closed database handle");
    }
}
}

