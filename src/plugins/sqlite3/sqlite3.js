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

var SQLite3 = exports.SQLite3;

// generators are easier to write in JS space
SQLite3.Cursor.prototype.__iterator__ = function() {
  while (true) {
    let row = this.next();
    if (row == null)
      throw StopIteration;
    yield row;
  }
};

// SQLite3 {{{
/**
 * Opens a handle to the database dsn Will usually be
 * a filename, but could also be ":memory:".
 *
 * @name SQLite3
 * @constructor
 *
 * @param dsn Path to sqlite DB file (or anything else it sqlite3_open
 *        supports)
 *
 * @see SQLite3.Cursor
 *
 * @class SQLite3 Database handle.
 *
 * ==Example:==
 * {{{
 * const SQLite3 = require('sqlite3').SQLite3;
 * var db = new SQLite3('./my_db.sqlite');
 * var c = db.cursor('SELECT * FROM table_1 WHERE id IN (?,?)');
 * c.bind([4,6]);
 *
 * for (let row in c) {
 *  print(row.toSource());
 * }
 * }}}
 * 
 */

// Class properties {{{
/**#@+ @fieldOf SQLite3 */

  /**
   * SQLite3 version as human readable string
   * @name versionStr
   * @type string
   */

  /**
   * SQLite3 version as an integer. For example, SQLite v3.6.12 would have a
   * version of 3006012
   * @name version
   * @type int
   */

/**#@- }}} */

// Class properties {{{
/**#@+ @methodOf SQLite3# */

  /**
   * Close the database handle. Force the database handle to be closed now,
   * instead of when the object gets garbage collected.
   * @name close
   */

  /**
   * Returns the row id of the last inserted column
   * @name lastInsertID
   * @function
   *
   * @returns a number
   */

  /**
   * Executes given SQL statement(s). Bind parameters can be passed to fill
   * placeholders used in the corresponding SQL statement.
   * 
   * If parameter sql_or_array is an array it is expected to be an array of 
   * objects with sql and optional bind properties. The bind property should
   * contain a value, an array or an object. The object can be used for named placeholders
   * In this case the parameter bind_arg is ignored!
   *
   * If parameter sql_or_array is a string it will be executed and the content of the 
   * optional parameter bind_arg will be used to fill placeholders.
   * 
   * ==Examples with a single statement:==
   *
   * {{{ 
   * db.exec('INSERT INTO foobar VALUES(?,?,?)',[1,2,3])
   * db.exec('INSERT INTO foobar VALUES(:first, :second, :third)',
   *         { ':first': 4, ':second': 5, ':third': 6 })
   * }}}
   * 
   * ==An example with multiple statements:==
   *
   * {{{ 
   * db.exec([{ 
   *        sql: 'CREATE TABLE foobar (a,b,c)'
   *    }, {
   *        sql: 'INSERT INTO foobar VALUES(?,?,?)',
   *        bind: [1,2,3]
   *    }, {
   *        sql: 'INSERT INTO foobar VALUES(:first, :second, :third)',
   *        bind: { ':first': 4, ':second': 5, ':third': 6 }
   *    }])
   * }}}
   *
   *
   * @name exec
   * @function
   *
   * @param sql_or_array SQL to prepare or an array of objects with sql and optional bind properties
   * @param bind_arg bind parameters passed to [[SQLite3.Cursor#bind]] (Optional).
   *
   * @returns number of executed statements
   * 
   */

  /**
   * Begin a transaction
   * @name begin
   * @function
   */

  /**
   * Commit a transaction
   * @name commit
   * @function
   */   

  /**
   * Rollback a transaction
   * @name rollback
   * @function
   */

  /**
   * Get a cursor to execute the given SQL statement. Bind parameters can be
   * passed in as parameters the SQL, or by using [[SQLite3.Cursor#bind]].
   * @name query
   * @function
   *
   * @param sql SQL to prepare.
   * @param bind_arg bind parameters passed to [[SQLite3.Cursor#bind]].
   *
   * @returns {SQLite3.Cursor} A cursor object
   */

/**#@- }}} */
// }}}

// SQLite3.Cursor {{{ 
/**
 * You cannot construct a cursor object directly, use [[SQLite3#cursor]] to
 * create one.
 *
 * @class Cursor class used to execute statements.
 * @name SQLite3.Cursor
 */

// Instance methods  {{{
/**#@+ @methodOf SQLite3.Cursor# */

  /**
   * Bind placeholder values. Accepts either an Array or an Object. SQLite
   * bind params are 1 based. However when an array is passed the first bind
   * param is pulled from the 0th element of the array, etc.
   *
   * @name bind
   *
   * @param binds bind parameters
   */

  /**
   * Get next row from this cursor. This is the alternative way of getting at
   * the results if you don't like the iterator style (or if the JS engine
   * doesn't support it).
   *
   * @returns Next row as an array, or null when end of results reached.
   *
   * @name SQLite3.Cursor.prototype.next
   */

  /**
   * Close this cursor.  Force it to be closed now, instead of waiting for it to
   * get garbage collected.
   *
   * You don't //have// to call this method, but its probably a good idea to
   * call if you know you wont need to use this cursor any more, since garbage
   * collection might take a long time to run.
   *
   * @name SQLite3.Cursor.prototype.close
   */

  /**
   * Reset the cursor back to the start.  This will also clear any errors.
   *
   * This does not clear any bound parameters.
   * 
   * @name reset
   */
/**#@- }}} */

// Instance properties {{{
/**#@+ @fieldOf SQLite3.Cursor# */

  /**
   * Iterator support.  A generator function that enables you to write
   *
   * {{{ 
   * for (row in myCursor) { ... }
   * }}}
   *
   * Each row will only be fetched from SQLite as needed as by [[#next]].
   * 
   * @name __iterator__
   */
/**#@- }}} */
// }}}

