// vim:ts=2:sw=2:expandtab:autoindent:
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

const asserts = require('test').asserts,
      sqlite3 = require('sqlite3'),
      sqlite3_testblob = require('binary').ByteString("0123456789");

if(!this.exports) this.exports = {};
if(!this.sqlite_test_helper) this.sqlite_test_helper = {}

this.sqlite_test_helper.get_db = function(){
    db = sqlite3.SQLite3(':memory:')
    db.exec('CREATE TABLE test_table(int_val INTEGER, str_val TEXT, bin_val BLOB)')    
    return db
}

this.sqlite_test_helper.get_test_statements = function(){
    data = [[ 1, "one",     sqlite3_testblob ],
            [ 2, "two",     sqlite3_testblob ],
            [ 3, "three",   sqlite3_testblob ],
            [ 4, "four",    sqlite3_testblob ],
            [ 5, "five",    sqlite3_testblob ],
            [ 6, "six",     sqlite3_testblob ],
            [ 7, "seven",   sqlite3_testblob ],
            [ 8, "eight",   sqlite3_testblob ]]
    
    return [{
            sql: 'INSERT INTO test_table VALUES(?,?,?)', 
            data: data[0],
            bind: data[0]
        },{
            sql: 'INSERT INTO test_table VALUES(:first,:second,:third)', 
            data: data[1],
            bind: { 
                first:  data[1][0], 
                second: data[1][1], 
                third:  data[1][2]
        }},{
            sql: 'INSERT INTO test_table VALUES(?1,?2,?3)', 
            data: data[2],
            bind: data[2]
        },{
            sql: 'INSERT INTO test_table VALUES($first,$second,$third)', 
            data: data[3],
            bind: {
                first:  data[3][0], 
                second: data[3][1], 
                third:  data[3][2]
        }},{
            sql: 'INSERT INTO test_table VALUES(@first,@second,@third)', 
            data: data[4],
            bind: { 
                first:  data[4][0], 
                second: data[4][1], 
                third:  data[4][2], 
        }},{
            sql: 'INSERT INTO test_table VALUES(:first,$second,@third)', 
            data: data[5], 
            bind: { 
                first:   data[5][0], 
                second:  data[5][1],  
                third:   data[5][2], 
        }},{
            sql: 'INSERT INTO test_table VALUES(:first,$second,@third)', 
            data: data[6],
            bind: { 
                ":first":   data[6][0],
                "$second":  data[6][1],
                "@third":   data[6][2],
        }},{
            sql: 'INSERT INTO test_table VALUES(?1,?2,?3)', 
            data: data[7],
            bind: data[7]
        }]
}

exports.sqlite_test_sqlite3_usage = function() {
    db = sqlite_test_helper.get_db()
}

exports.test_sqlite3_placeholder_exec = function() {
    db = sqlite_test_helper.get_db()
    data = sqlite_test_helper.get_test_statements()
    for(idx in data){
        db.exec(data[idx].sql, data[idx].bind)
    }
}

exports.test_sqlite3_placeholder_execMany = function(db) {
    db = sqlite_test_helper.get_db()
    db.execMany(sqlite_test_helper.get_test_statements())
}

exports.test_sqlite3_placeholder_query_row_array = function() {
    db = sqlite_test_helper.get_db()
    data = sqlite_test_helper.get_test_statements()
    
    db.execMany(data)
    cur = db.query("SELECT * from test_table")
    index = 0
    while((row = cur.next()) != null){
        asserts.same(row[0], data[index].data[0])
        asserts.same(row[1], data[index].data[1])
        asserts.same(row[2].toArray(), data[index].data[2].toArray())
        ++index
    }
    asserts.same(index, data.length)    
}

exports.test_sqlite3_placeholder_query_row_object = function() {
    db = sqlite_test_helper.get_db()
    data = sqlite_test_helper.get_test_statements()
    
    db.execMany(data)
    cur = db.query("SELECT * from test_table")
    index = 0
    while((row = cur.next(true)) != null){
        asserts.same(row.int_val, data[index].data[0])
        asserts.same(row.str_val, data[index].data[1])
        asserts.same(row.bin_val.toArray(), data[index].data[2].toArray())
        ++index
    }
    asserts.same(index, data.length)    
}

require('test').runner(exports);

