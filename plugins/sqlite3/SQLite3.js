// vim:ts=2:sw=2:expandtab:autoindent:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

(function() {
  $importer.load('SQLite3', true);

  var SQLite3 = $importer.context.SQLite3;

  // generators are easier to write in JS space
  SQLite3.Cursor.prototype.__iterator__ = function() {
    while (true) {
      let row = this.next();
      if (row == null)
        throw StopIteration;
      yield row;
    }

  };

  return SQLite3;
})()


/***
 * Title: sqlite3
 *
 * Package for SQLite3 DBMS
 *
 * Class: SQLite3 
 *
 *  SQLite3 Database handle
 *
 * Synopsis:
 *
 * (code)
 * import('sqlite3');
 * var db = new SQLite3('./my_db.sqlite');
 * var c = db.cursor('SELECT * FROM table_1 WHERE id IN (?,?)');
 * c.bind([4,6]);
 * 
 * for (let row in c) {
 *   print(row.toSource());
 * }
 * (end)
 * 
 * Group: Constructor Properties
 *
 * str: versionStr
 * SQLite3 version as human readable string
 *
 * int: version
 * SQLite3 
 */


/***
 * Group: Constructor
 *
 * Constructor: SQLite3
 *
 * Parameters:
 *  dsn - Path to sqlite DB file (or anything else it sqlite3_open supports) 
 *
 * Opens a handle to the database dsn Will usually be a filename, but could
 * also be ":memory:".
 */

/***
 * Group: Methods
 *
 * Function: close
 *
 * Close the database handle. Force the database handle to be closed now,
 * instead of when the object gets garbage collected.
 *
 * Function: cursor
 *
 * Get a cursor to execute the given SQL statement. Bind parameters can be
 * passed in as parameters the SQL, or by using <Cursor.bind>.
 *
 * Parameters:
 *  sql - SQL to prepare
 *  bind_arg - bind parameters passed to <Cursor.bind>.
 *
 * Returns:
 *  A <SQLite3.Cursor> object
 */

/***
 * Class: SQLite3.Cursor
 *
 * Cursor class use to execute statements. You cannot construct a cursor
 * object directly, use <SQLite3.cursor> to create one.
 */

/***
 * Group: Methods
 *
 * Function: bind
 *
 * Bind placeholder values. Accepts either an Array or an Object.
 *
 *
 * Parameters:
 *  binds - bind parameters
 *
 * Example: 
 * 
 * (example)
 * cursor.bind( ['foo', 'bar', 'baz' ] );
 *
 * // Note: one-based, not zero-based when using object!
 * cursor.bind( { 
 *   1: 'foo', 
 *   2: 'bar', 
 *   3: 'baz' 
 * } ); 
 *
 * // Named params
 * cursor = db.cursor("... WHERE foo = >foo AND bar = ?bar OR baz < $baz
 * cursor.bind( { 
 *   '>foo': 'foo', 
 *   '?bar': 'bar', 
 *   '$baz': 3 
 * } ); 
 * (end)
 *
 * SQLite bind params are 1 based. However when an array is passed the first
 * bind param is pulled from the 0th element of the array, etc.
 */

/***
 * Function: next
 *
 * Get next row from this cursor.
 *
 * Returns:
 *  Next row as an array, or null when end of results reached.
 */


/***
 * Function: close
 *
 * Close this cursor. Force it to be closed now, instead of waiting for it
 * to get garbage collected.
 *
 * You don't have to call this method, but its probably a good idea to call
 * if you know you wont need to use this cursor any more.
 */

/***
 * Function: reset
 *
 * Reset the cursor back to the start. This will also clear any errors.
 *
 * This does not clear any bound parameters.
 */

/***
 * Function: __iterator__
 *
 * Iterator support. A generator function that enables you to write
 * 
 * > for (row in c) { ... }
 *
 * Each row will only be fetched from SQLite as needed.
 */
