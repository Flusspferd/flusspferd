const encodings = require('encodings'),
      asserts = require('test').asserts,
      util = require('util');

require.paths.unshift('test/js/lib');

if (!this.exports) this.exports = {};

exports.test_generatorsInMoudles = function() {
  var a = require('./generator_test').run();

  var it = (k for (k in [1,2,3]) );
  asserts.same(a.toString(), "[object Generator]", "got a generator");
  asserts.same(typeof it.next, typeof a.next, "both generators have a 'next' method");
}

exports.test_utilRange = function() {
  var r = Util.Range(1,10,2);
  var a1 = [];

  for (let i in r) { a1.push(i) };

  asserts.same(a1, [1,3,5,7,9], "Util.Range works");
}

require('test').runner(exports);
