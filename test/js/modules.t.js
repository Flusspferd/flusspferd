let test = require('test'),
    asserts = test.asserts;


exports.test_RequireId = function () {
  asserts.matches(require.id, "file://.*/test/js/modules.t.js$");
}

exports.test_InstanceOf = function () {
  var a1 = require('./lib/modules-test/a1');
  var a2 = require('./lib/modules-test/a2');
  asserts.instanceOf(a1.array(), Array, "Array in a1 is instanceof main Array");
  asserts.instanceOf(a2.array(), Array, "Array in a2 is instanceof main Array");
  asserts.instanceOf([1,2,3], Array, "sanity check");
}

var global = this;
exports.test_varPolution = function() {
  require('./lib/modules-test/a2');
  asserts.same("a2" in global, false, "no a2 variable");
}

if (require.main === module)
  test.prove(module.id);
