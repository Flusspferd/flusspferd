const asserts = require('test').asserts;

exports.test_outer = {
  test_inner1: function () {
    asserts.ok(true, "inner1_1");
    asserts.ok(true, "inner1_2");
  },
  test_inner2: function () {
    asserts.ok(true, "inner2_1");
    asserts.ok(true, "inner2_2");
  }
};
exports.test_notNested = function () {
  asserts.ok(true, "Non-nested tests called too");
}

if (module == require.main)
  require('test').runner(exports);
