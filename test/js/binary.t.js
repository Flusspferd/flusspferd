const binary = require('binary');
const asserts = require('test').asserts;

if (!this.exports) this.exports = {};

exports.test_decodeToString = function() {
	var b = binary.ByteString([65, 66]);
	asserts.same(b.decodeToString(), "AB");
}

if (require.main === module)
  require('test').runner(exports);
