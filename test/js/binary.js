const binary = require('binary');
const asserts = require('test').asserts;

if (!this.exports) this.exports = {};

exports.test_decodeToString = function() {
	b = binary.ByteString([65, 66]);
	asserts.same(b.decodeToString(), "AB");	
}

require('test').runner(exports);
