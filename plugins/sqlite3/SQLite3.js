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
  Import('SQLite3', true);

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
 * @class SQLite3 Database handle.
 *
 * <br /><br />
 *
 * Example usage: 
 * <pre class="code">
 * Import('SQLite3');
 * var db = new SQLite3('./my_db.sqlite');
 * var c = db.cursor('SELECT * FROM table_1 WHERE id IN (?,?)');
 * c.bind([4,6]);
 *
 * for (let row in c) {
 *  print(row.toSource());
 * }
 * </pre>
 * 
 */

/**
 * SQLite3 version as human readable string
 * @name versionStr
 * @type string
 * @fieldOf SQLite3
 */

/**
 * SQLite3 version as an integer. For example, SQLite v3.6.12 would have a
 * version of 3006012
 * @name version
 * @type int
 * @fieldOf SQLite3
 */

/**
 * Close the database handle. Force the database handle to be closed now,
 * instead of when the object gets garbage collected.
 * @name close
 * @methodOf SQLite3.prototype
 */

/**
 * Get a cursor to execute the given SQL statement. Bind parameters can be
 * passed in as parameters the SQL, or by using {@link Cursor#bind}.
 * @name cursor
 * @function
 * @methodOf SQLite3.prototype
 *
 * @param sql SQL to prepare.
 * @param bind_arg bind parameters passed to {@link Cursor#bind}.
 *
 * @returns {SQLite3.Cursor} A cursor object
 */

/**
 * You cannot construct a cursor object directly, use {@link SQLite3#cursor} to
 * create one.
 *
 * @class Cursor class used to execute statements.
 * @name SQLite3.Cursor
 */

// TODO: Sort out the JSDoc template to deal with multiple paras
/**
 * Bind placeholder values. Accepts either an Array or an Object. SQLite
 * bind params are 1 based. However when an array is passed the first bind
 * param is pulled from the 0th element of the array, etc.
 *
 * <br/><br/>
 * 
 * SQLite bind params are 1 based.  However when an array is passed the first
 * bind param is pulled from the 0th element of the array, etc.
 *
 * @name bind
 * @methodOf SQLite3.Cursor.prototype
 *
 * @param binds bind parameters
 */

/**
 * Get next row from this cursor.
 *
 * @returns Next row as an array, or null when end of results reached.
 *
 * @name next
 * @methodOf SQLite3.Cursor.prototype
 */

/**
 * Close this cursor.  Force it to be closed now, instead of waiting for it to
 * get garbage collected.
 *
 * <br /><br />
 *
 * You don't <b>have</b> to call this method, but its probably a good idea to
 * call if you know you wont need to use this cursor any more, since garbage
 * collection might take a long time to run.
 *
 * @name SQLite3.Cursor.prototype.close
 * @methodOf SQLite3.Cursor.prototype
 */

/**
 * Reset the cursor back to the start.  This will also clear any errors.
 *
 * <br /><br />
 *
 * This does not clear any bound parameters.
 * 
 * @name reset
 * @methodOf SQLite3.Cursor.prototype
 */

/**
 * Iterator support.  A generator function that enables you to write
 *
 * <pre class="code">
 * for (row in myCursor) { ... }
 * </pre>
 *
 * Each row will only be fetched from SQLite as needed.
 * 
 * @name __iterator__
 * @methodOf SQLite3.Cursor.prototype
 */
