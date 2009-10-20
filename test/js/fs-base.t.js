const fs = require('filesystem-base'),
      asserts = require('test').asserts;

exports.test_canonical = function() {
  // This isn't the most ideal test, but its better than nothing
  asserts.ok(
    fs.canonical('..') != fs.canonical('.'),
    ".. and . canonicalize to different things"
  );

  asserts.ok(
    fs.canonical('..') != fs.canonical('../..'),
    ".. and ../.. canonicalize to different things"
  );
}

if (require.main === module)
  require('test').runner(exports);
