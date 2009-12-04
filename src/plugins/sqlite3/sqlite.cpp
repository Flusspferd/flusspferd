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
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/fusion/include/make_vector.hpp>

using namespace flusspferd;
using namespace boost::assign;
namespace fusion = boost::fusion;

namespace sqlite3_plugin {

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
void sqlite3::query(call_context &x) {    
    local_root_scope scope;
    ensure_opened();

    if (x.arg.size() < 1) {
        throw exception ("SQLite3.query() at least 1 argument");
    }

    value bind;
    if ( x.arg.size() > 1 ) {
        bind = x.arg[1];
    }

    x.result = compile( x.arg[0].to_string(), bind);
}

///////////////////////////
void sqlite3::exec(call_context & x) {
    local_root_scope scope;

    if (x.arg.size() != 1 && x.arg.size() != 2) {
        throw exception ("SQLite3.exec() requires at least 1 argument");
    }
    
    if ( x.arg[0].is_string() ) {
        object data = create<object>();
        data.set_property("sql", x.arg[0].get_string() );        

        if ( x.arg.size() > 1 ) {
            data.set_property( "bind", x.arg[1].to_object() );
        }

        array arr(create<array>(list_of(data)));
        x.result = exec_internal( arr );
    }    
    else {
        throw exception("The first parameter of SQLite3.exec() requires to be"
                        " a string containing an SQL statement");
    }
}

///////////////////////////
void sqlite3::execMany(call_context & x) {
    local_root_scope scope;

    if (x.arg.size() != 1) {
        throw exception ("SQLite3.execMany() requires 1 argument");
    }

    if ( x.arg[0].is_object() && x.arg[0].get_object().is_array() ) {
        x.result = exec_internal( x.arg[0].to_object() );
    }
    else {
        throw exception("SQLite3.execMany() expected an array of"
                        " objects as argument");
    }

}

///////////////////////////
int sqlite3::exec_internal( array arr ) {
    local_root_scope scope;

    int count = 0; 
    for ( size_t idx = 0; idx < arr.size(); ++idx ) {
        value const & v = arr.get_element(idx);
        
        if ( !v.is_object() ) {
            throw exception("exec expects an object");
        }

        object obj = v.get_object();

        if ( !obj.has_property("sql") ) {
            throw exception("no sql statement found");
        }
        
        value bind;
        if ( obj.has_property("bind") ){
            bind = obj.get_property("bind");
        }

        object result = compile( obj.get_property("sql").to_string(), bind); 
        
        result.call("next");

        count += sqlite3_changes(db);
    }
    
    return count;
}

///////////////////////////
object sqlite3::compile(flusspferd::string sql_in, value bind ) {
    local_root_scope scope;

    size_t n_bytes = sql_in.length() * 2;
    sqlite3_stmt * sth = 0;
    char16_t * tail = 0; // uncompiled part of the sql (when multiple stmts)
    
    if (sqlite3_prepare16_v2(db, sql_in.data(), n_bytes, &sth, (const void**)&tail) != SQLITE_OK)
    {
        raise_sqlite_error(db);
    }

    object cursor = create<sqlite3_cursor>(fusion::make_vector(sth));

    string tail_str;
    if (tail) {
        tail_str = string(tail);
    }
    
    string sql = sql_in.substr( 0, sql_in.size() - tail_str.size() );

    cursor.define_property("sql", sql);
    cursor.define_property("tail", tail_str);        

    if ( !bind.is_undefined_or_null() ) {
        cursor.call("bind", bind );
    }
    
    return cursor;
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

///////////////////////////
void sqlite3::begin() {
    ensure_opened();

    if ( sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, 0) != SQLITE_OK ) {
         raise_sqlite_error(db);
    }
}

///////////////////////////
void sqlite3::commit() {
    ensure_opened();

    if ( sqlite3_exec(db, "COMMIT TRANSACTION", 0, 0, 0) != SQLITE_OK ) {
         raise_sqlite_error(db);
    }
}

///////////////////////////
void sqlite3::rollback() {    
    ensure_opened();

    if ( sqlite3_exec(db, "ROLLBACK TRANSACTION", 0, 0, 0) != SQLITE_OK ) {
        raise_sqlite_error(db);
    }
}

}

