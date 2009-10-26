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

#include "sqlite_cursor.hpp"
#include "sqlite.hpp"
#include <sstream>
#include <new>
#include <boost/lexical_cast.hpp>


namespace sqlite3_plugin {

using namespace flusspferd;

///////////////////////////
// 'Private' constructor that is called from sqlite3::cursor
sqlite3_cursor::sqlite3_cursor(object const &obj, sqlite3_stmt *_sth)
: base_type(obj)
, sth(_sth)
, state(CursorState_Init)
, param_bound( sqlite3_bind_parameter_count(_sth), false )
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
void sqlite3_cursor::next(call_context & x) {
    local_root_scope scope;

    if (!sth){
        throw exception("SQLite3.Cursor.next called on closed cursor");
    }

    switch (state) {
        case CursorState_Finished:
            // We've seen the last row, remember it and return the 
            // EOF indicator
            x.result = object();
            return;
        case CursorState_Errored:
            throw exception("SQLite3.Cursor: This cursor has seen an error and"
                            " needs to be reset");
        default:
            break;    
    }

    ensure_all_params_bound();

    int code = sqlite3_step(sth);

    if (code == SQLITE_DONE) {
        state = CursorState_Finished;
        x.result = object();
        return;
    } else if (code != SQLITE_ROW) {
        if (sqlite3_errcode( sqlite3_db_handle(sth) ) != SQLITE_OK) {
            state = CursorState_Errored;
            raise_sqlite_error();
        } else {
            throw exception("SQLite3.Cursor: database reported misuse error. "
                            "Please try again");
        }
    }

    state = CursorState_InProgress;

    if ( x.arg.size() == 1 && x.arg[0].is_boolean() && x.arg[0].get_boolean() ) {
        x.result = create_result_object();
    }
    else {
        x.result = create_result_array();
    }
}

///////////////////////////
object sqlite3_cursor::create_result_array() {
    local_root_scope scope;
    // Build up the row object.
    int cols = sqlite3_column_count(sth);
    array row = create<array>();
    for (int i=0; i < cols; i++)
    {
        row.set_element(i, get_column(i) );
    }
    return row;    
}

///////////////////////////
object sqlite3_cursor::create_result_object() {
    local_root_scope scope;
    // Build up the row object.
    int cols = sqlite3_column_count(sth);
    object row = create_object();
    for (int i=0; i < cols; i++)
    {
        char16_t const * name_str = reinterpret_cast<char16_t const *>( sqlite3_column_name16(sth, i) );
        if ( !name_str ) {
            throw exception("Couldn't retrieve column name for column");
        }
        string name = std::basic_string<char16_t>(name_str);
        row.set_property( name, get_column(i) );
    }
    return row;    
}

///////////////////////////
value sqlite3_cursor::get_column(int i) {
    local_root_scope scope;
    
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
        case SQLITE_BLOB:
        {
            unsigned char const * bytes = reinterpret_cast<unsigned char const*>(sqlite3_column_blob(sth, i));
            if (!bytes) {
                raise_sqlite_error();
            }

            size_t length = sqlite3_column_bytes(sth, i);
            col = create_native_object<byte_string>(object(), bytes, length);
        }
        break;
        case SQLITE_TEXT:
        {
            char16_t *text  = (char16_t*)sqlite3_column_text16(sth, i);
            
            if (!text) {
                // Checking if it was an allocation error if so we throw
                raise_sqlite_error();
            }
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
    return col;
}

///////////////////////////
// JS method
void sqlite3_cursor::bind(call_context &x) {
    local_root_scope scope;

    size_t num_binds = sqlite3_bind_parameter_count(sth);

    if (!num_binds) {
        return;
    }
  
    if (x.arg[0].is_object()) {
        object o = x.arg[0].get_object();
    
        if (o.is_array()) {
            array a = o;
            bind_array(a, num_binds);
        }
        else if ( is_native<binary>(o) ) {
            do_bind_param(1, x.arg[0]);
        }
        else {
            bind_dict(o, num_binds);
        }
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
        do_bind_param(n, bind);
    }
}

///////////////////////////
// bind named or numbered params
void sqlite3_cursor::bind_dict(object &o, size_t num_binds) {

    for (size_t n = 1; n <= num_binds; n++) {
        const char * name = sqlite3_bind_parameter_name(sth, n);
        bool found = false;
        value bind;

        if (!name) {
            // Possibly a '?' unnnamed param
            // TODO: This will break when n is > 2^31.
            if( o.has_property( value(n) ) ) {
                found = true;
                bind = o.get_property( value(n) );
            }
        } else {
            // Named param
            if( o.has_property(name) ) {
                found = true;
                bind = o.get_property(name);
            }
            // If not found the property we did not find any suitable parameter with this name
            // So we'll try if we find a property which has the name without the first character
            // and only if the name starts with '@',':' or '$' 
            else {
                switch(name[0])
                {
                case '@':
                case '$':
                case ':':
                    if( o.has_property(++name) ) {
                        found = true;
                        bind = o.get_property(name);    
                    }
                }                
            }            
        }
        
        // if found now is false there was no suitable bind parameter specified 
        // so we do not call do_bind_param because the user might want to bind
        // later
        if (found) {
            do_bind_param(n, bind);
        }
  }
}

///////////////////////////
// Bind the actual para
void sqlite3_cursor::do_bind_param(int n, value v) {
    int ok;

    if (v.is_undefined()){
        char const * name = sqlite3_bind_parameter_name(sth, n);
        throw exception("SQLite3.Cursor.bind() attempt to bind undefined value to placeholder " +
                            boost::lexical_cast<std::string>(n) + 
                            (name ? " '" + std::string(name) + "'" : std::string()) );
    }
    else if (v.is_int()) {
        ok = sqlite3_bind_int(sth, n, v.get_int());
    } else if (v.is_double()) {
        ok = sqlite3_bind_double(sth, n, v.get_double());
    } else if (v.is_null()) {
        ok = sqlite3_bind_null(sth, n);                
    } else if ( v.is_object() && is_native<binary>(v.get_object()) ) {
        binary & b = flusspferd::get_native<binary>(v.get_object());        
        binary::vector_type const & vec = b.get_const_data();
        ok = sqlite3_bind_blob( sth, n, (vec.empty() ? 0 : &vec[0]), vec.size(), SQLITE_TRANSIENT );
    } else {
        // Default, stringify the object
        string bind = v.to_string();
        ok = sqlite3_bind_text16(sth, n, bind.data(), bind.length()*2, SQLITE_TRANSIENT);
    }    

    if (ok != SQLITE_OK) {
        raise_sqlite_error(); 
    }

    if(n > 0 && size_t(n) <= param_bound.size())
    {
        param_bound[ size_t(n - 1) ] = true;
    }
}

bool sqlite3_cursor::all_params_bound() const {
    for(size_t i = 0; i < param_bound.size(); ++i){
        if(!param_bound[i]){
            return false;
        }
    }
    return true;
}

void sqlite3_cursor::ensure_all_params_bound() const {
    for(size_t i = 0; i < param_bound.size(); ++i){
        if(!param_bound[i]){
            throw exception("SQLite3() not all placeholders bound on executed statement!");
        }
    }    
}

void sqlite3_cursor::raise_sqlite_error() {
    sqlite3_plugin::raise_sqlite_error( sqlite3_db_handle(sth) );
}


}

