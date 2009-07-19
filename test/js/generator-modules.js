
// Run me with
//   ./util/jsrepl.sh -I test/js/lib/ test/js/generator-modules.js 
const encodings = require('encodings'),
      asserts = require('test').asserts;


if (!this.exports) this.exports = {};
exports.test_generatorsInMoudles = function() {
  var a = require('./generator_test').run();

  var it = (k for (k in [1,2,3]) );
  asserts.same(a.toString(), "[object Generator]", "got a generator");
  asserts.same(it.next, a.next, "both generators have a 'next' method");
}


require('test').runner(exports);
