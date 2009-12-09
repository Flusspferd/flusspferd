const cURL = require('curl');
const asserts = require('test').asserts;

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

if (require.main === module)
  require('test').runner(exports);
