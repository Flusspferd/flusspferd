// flusspferd: -I foo/bar/baz
const test = require('test');

// This test only works when we are run as the 'main' file, so skip otherwise
if (require.main === module) {

  exports.test_shebangHandling = function() {
    test.asserts.same(require.paths[0], "foo/bar/baz", "require path added from opt-line");
  }

  test.runner(exports);
}
else {
  exports.test_shebangHandling = function() {
    test.asserts.ok(true, "SKIPPED - can only test when run as main module");
  }
}
