let test = require('test'),
    asserts = test.asserts;


exports.test_RequireId = function () {
  asserts.matches(require.id, "file://.*/test/js/modules.t.js$");
}

exports.test_InstanceOf = function () {
  var a1 = require('./lib/modules-test/a1');
  // If this require fails complaing about const redefined, it means |const x|
  // is happening on the global varobj, not one for a module
  var a2 = require('./lib/modules-test/a2');

  asserts.instanceOf(a1.array(), Array, "Array in a1 is instanceof main Array");
  asserts.instanceOf(a2.array(), Array, "Array in a2 is instanceof main Array");
  asserts.instanceOf([1,2,3], Array, "sanity check");
}

// Check that when one module includes another, both keep the right require.id,
// even after the initial call form require has returned
exports.test_require_id = function () {
  var a1 = require('./lib/modules-test/a1');
  asserts.matches(a1.id, "file://.*/a1.js$");
  asserts.matches(a1.id_now(), "file://.*/a1.js$");

  var a2 = require('./lib/modules-test/a2');
  asserts.matches(a2.id,       "file://.*/a2.js$");
  asserts.matches(a2.id_now(), "file://.*/a2.js$");
}

var global = this;
exports.test_varPolution = function() {
  require('./lib/modules-test/a2');
  asserts.same("a2" in global, false, "no a2 variable");
}

exports.test_module_resouce = function() {
  asserts.matches(module.resource.resolve("x.ext"), ".*/test/js/x.ext$");
  var a1 = require('./lib/modules-test/a1');
  asserts.diag(a1.module);
  asserts.matches(a1.module.resource.resolve("x.ext"), ".*/test/js/lib/modules-test/x.ext$");

  // Check we can actually open things too!
  asserts.same(a1.module.resource("a1.js").readLine(), "// First line of a1.js\n");
}

// Test that you can load DSO modules via relative require. This test might be
// better if we had a dedicated DSO module to load. Since right now we dont
// always have any built :(
//
// This test is also fragile: it's dependant on where exactly modules built
// to
exports.test_relative_dso = function() {
  try {
    var curl = require('curl');
  }
  catch (e) {
    if (e.match(/^Unable to load module 'curl' /))
      return;
  }

  asserts.same(require('../../build/modules/curl'), curl,
               "Can load curl DSO by relative include");
}

if (require.main === module)
  test.prove(module.id);
