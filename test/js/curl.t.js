const cURL = require('curl');
const asserts = require('test').asserts;

exports.test_curlCleanup = function() {
  cURL.globalInit(cURL.GLOBAL_ALL);
  var c = new cURL.Easy();
  asserts.ok(c.valid());
  c.cleanup();
  asserts.ok(!c.valid());
};

exports.test_curlReset = function() {
  cURL.globalInit(cURL.GLOBAL_ALL);
  var c = new cURL.Easy();
  asserts.ok(c.valid());
  c.options.url = 'foo';
  asserts.same(c.options.url, 'foo');
  c.reset();
  asserts.ok(c.valid());
  asserts.same(c.options.url, '');
};

exports.test_curlEncode = function() {
  cURL.globalInit(cURL.GLOBAL_ALL);
  var c = new cURL.Easy();
  asserts.ok(c.valid());
  var input = 'Hellö Wörld!';
  var coded = c.escape(input);
  var result = c.unescape(coded);
  asserts.same(input, result);
};

if (require.main === module)
  require('test').runner(exports);
