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

/** section: Bundled Modules
 * sqlite3
 *
 *  SQLite3 database bindings. [[sqlite3.SQLite3]] is the main class.
 *
 *  ##### Example: #
 *
 *      const SQLite3 = require('sqlite3').SQLite3;
 *      var db = new SQLite3('./my_db.sqlite');
 *      var c = db.cursor('SELECT * FROM table_1 WHERE id IN (?,?)');
 *      c.bind([4,6]);
 *
 *      for (let row in c) {
 *       print(row.toSource());
 *      }
 **/

if(!exports.SQLite3) {
  // The only time this will be the case is when in the dev REPL where the JS
  // in is the search path but the module was not built.
  throw new Error("Unable to find library 'sqlite3'. (No native module found)");
}

/**
 *  class sqlite3.SQLite3
 *
 *  SQLite3 is a self-contained, single file based RDBMS. See [[sqlite3]] for an
 *  short example.
 **/
// Note - most of the SQLite3 class is C++
var SQLite3 = exports.SQLite3;

/**
 *  new sqlite3.SQLite3(dsn)
 *  - dsn (String): Path to database file, or ':memory:'
 *
 *  Opens a handle to the database `dsn`, which will usually be a filename, but
 *  could also be ":memory:", or any other special string understood by SQLite3.
 **/

/**
 *  sqlite3.SQLite3.versionStr -> String
 *
 *  SQLite3 library version as a human readable string
 **/

/**
 *  sqlite3.SQLite3.version -> Number
 *
 *  SQLite3 version as an integer. For example, SQLite v3.6.12 would have a
 *  version of 3006012
 **/

/**
 *  sqlite3.SQLite3#close() -> undefined
 *
 *  Close the database handle. Force the database handle to be closed now,
 *  instead of when the object gets garbage collected.
 **/

/**
 *  sqlite3.SQLite3#lastInsertID() -> Number
 *
 *  Returns the row id of the last inserted auto-increment column.
 **/

/**
 *  sqlite3.SQLite3#exec(sql[, binds]) -> Number
 *  - sql (String): SQL to execute
 *  - binds (Array | Object): parameters to bind into `sql`
 *
 *  Executes a given SQL statement and optionally, bind parameters can be passed
 *  to fill the placeholders in the SQL statement. Returns the number of rows
 *  affected by `sql`
 *
 *  *Note:* If the execution throws an exception and the execution is
 *  aborted it will not automatically rollback the applied changes. If this is
 *  wanted the user of this function has to explicitly take care about it by
 *  using [[sqlite3.SQLite3#begin]],[[sqlite3.SQLite3#commit]] and
 *  [[sqlite3.SQLite3#rollback]].
 *
 *  ##### Example: #
 *
 *      db.exec('CREATE TABLE foobar (a,b,c)');
 *      db.exec('INSERT INTO foobar VALUES(?,?,?)',[1,2,3]);
 *      db.exec('INSERT INTO foobar VALUES(:first, :second, :third)',
 *              { first: 4, second: 5, third: 6 });
 *
 **/

/**
 *  sqlite3.SQLite3#execMany(statements) -> Number
 *  - statements (Array): an Array of objects with keys `sql` and `bind`
 *
 *  Executes given SQL statement(s). Bind parameters can be passed to fill
 *  placeholders used in the corresponding SQL statement.
 *
 *  The parameter is expected to be an array of objects with sql and optional
 *  bind properties. The bind property should contain a value, an array or an
 *  object. The object can be used for named placeholders
 *
 * *Note:* If the execution throws an exception and the execution is aborted
 * it will not automatically rollback the applied changes. If this is wanted
 * the user of this function has to explicitly take care about it by using
 * [[SQLite3#begin]],[[SQLite3#commit]] and [[SQLite3#rollback]].
 *
 * ##### Example #
 *
 *      db.exec([
 *        { sql: 'CREATE TABLE foobar (a,b,c)'
 *        },
 *        { sql: 'INSERT INTO foobar VALUES(?,?,?)',
 *          bind: [1,2,3]
 *        },
 *        { sql: 'INSERT INTO foobar VALUES(:first, :second, :third)',
 *          bind: { first: 4, second: 5, third: 6 }
 *        }
 *      ])
 **/

/**
 * sqlite3.SQLite3#begin() -> undefined
 *
 * Begin a transaction. See [SQLite3 Transactions][sqlite3_txn] for details. This method is similar to running `dbh.exec('BEGIN WORK')`.
 *
 * [sqlite3_txn]: http://www.sqlite.org/lang_transaction.html
 **/

/**
 * sqlite3.SQLite3#commit() -> undefined
 *
 * Commit the current transaction.
 **/

/**
 * sqlite3.SQLite3#rollback() -> undefined
 *
 * Rollback the current transaction without writing the changes to disk.
 **/

/**
 *  sqlite3.SQLite3#query(sql[, bind]) -> sqlite3.SQLite3.Cursor
 *  - sql (String): SQL statement to prepare
 *  - bind (Object | Array): bind parameters for `sql`.
 *
 *  Get a cursor to execute the given SQL statement. Bind parameters can be
 *  set at cursor creqation time by passing them in as the second argument, or
 *  later by using [[sqlite3.SQLite3.Cursor#bind]].
 **/


/**
 *  class sqlite3.SQLite3.Cursor
 *
 *  Class used to execute SQL statements, usually ones returning data.
 *
 *  You cannot construct a cursor object directly, use [[sqlite.SQLite3#query]] to
 *  create one.
 **/

/**
 *  sqlite3.SQLite3.Cursor#bind(params) -> undefined
 *  - params ( Object | Array ): parameters to bind to the placeholders in the
 *    SQL
 *
 *  Bind placeholder values. Accepts either an Array or an Object. SQLite
 *  bind params are 1 based. However when an array is passed the first bind
 *  param is pulled from the 0th element of the array, etc.
 **/

/**
 *  sqlite3.SQLite3.Cursor#next([want_object]) -> Array | Object
 *  - want_object (Boolean): get the next row as an object or as an array
 *
 *  Get next row from this cursor. This is the alternative way of getting at
 *  the results if you don't like the iterator style (or if the JS engine
 *  doesn't support it).
 *
 *  If you pass as optional parameter true next will return an Object with the
 *  column names as id. For no parameter or anything else it will return an
 *  array.
 **/

/**
 *  sqlite3.SQLite3.Cursor#close() -> undefined
 *
 *  Close this cursor.  Force it to be closed now, instead of waiting for it to
 *  get garbage collected.
 *
 *  You don't _have_ to call this method, but its probably a good idea to call
 *  if you know you wont need to use this cursor any more, since garbage
 *  collection might take a long time to run.
 **/

/**
 *  sqlite3.SQLite3.Cursor#reset() -> undefined
 *
 *  Reset the cursor back to the start.  This will also clear any errors.
 *
 *  This does not clear any bound parameters.
 **/

/**
 *  sqlite3.SQLite3.Cursor#__iterator__() -> Iterator
 *
 *  Iterator support for Spidermonkey's Iterator/Generators. This behaviour
 *  lets you write:
 *
 *      for (row in myCursor) { ... }
 *
 *  and each row will only be fetched from SQLite as needed, by calling [[sqlite3.SQLite3.Cursor#next]].
 **/

// generators are easier to write in JS space
SQLite3.Cursor.prototype.__iterator__ = function() {
  while (true) {
    let row = this.next();
    if (row == null)
      throw StopIteration;
    yield row;
  }
};

SQLite3.prototype.cursor = function() {
  require('system').stderr.print('SQLite3.cursor() has been deprecated. use SQLite3.query() instead');
  return this.query.apply(this, arguments);
}
