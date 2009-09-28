require.paths.unshift('test/js/lib/modules-test');

let test = require('test'),
    asserts = test.asserts;


exports.test_RequireId = function () {
  // Not sure what require.id should be. it ceratinly shouldn't be 'system'
  asserts.same(require.id, "test/js/modules.t.js");
}

exports.test_InstanceOf = function () {
  var a1 = require('a1');
  var a2 = require('a2');
  asserts.instanceOf(a1.array(), Array, "Array in a1 is instanceof main Array");
  asserts.instanceOf(a2.array(), Array, "Array in a2 is instanceof main Array");
  asserts.instanceOf([1,2,3], Array, "sanity check");
}

var global = this;
exports.test_varPolution = function() {
  require('a2');
  asserts.same("a2" in global, false, "no a2 variable");
}

if (require.main === module)
  test.runner(exports);
