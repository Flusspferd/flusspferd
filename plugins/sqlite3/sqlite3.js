(function() {
  $importer.load('sqlite3', true);

  var SQLite3 = $importer.SQLite3;

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
