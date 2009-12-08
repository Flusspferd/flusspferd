const util = require('util');
const asserts = require('test').asserts;

exports.test_util_load_file = function() {
  asserts.same(
      util.load('test/js/util-load-file.js'),
      9);
}

if (require.main === module)
  require('test').runner(exports);
