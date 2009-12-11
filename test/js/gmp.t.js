const gmp = require('gmp');
const asserts = require('test').asserts;

exports.testInteger = function() {
  var j = gmp.Integer(10);
  asserts.same(j.toInt(), 10);
  asserts.same(j.toString(), '10');
  asserts.same(j.toString(2), '1010');
  var z = gmp.Integer(1);
  asserts.same(z.toInt(), 1);
  asserts.same(z.toString(), '1');
  asserts.same(z.toString(2), '1');
  var r = j.add(z);
  asserts.same(r.toInt(), 11);
  asserts.same(r.toString(), '11');
  asserts.same(r.toString(2), '1011');
  z = gmp.Integer(-5);
  asserts.same(z.toInt(), -5);
  asserts.same(z.toString(), '-5');
  asserts.same(z.toString(2), '-101');
  r = j.add(z);
  asserts.same(r.toInt(), 5);
  asserts.same(r.toString(), '5');
  asserts.same(r.toString(2), '101');
  r = z.abs();
  asserts.same(r.toInt(), 5);
  asserts.same(r.toString(), '5');
  asserts.same(r.toString(2), '101');
};

exports.testFloat = function() {
  var f = gmp.Float(10);
  asserts.same(f.toInt(), 10);
  asserts.same(f.toString(), '10.0');
  asserts.same(f.toString(2), '1010.0');
  asserts.ok(f.fitsInt());
};

if (require.main === module)
  require('test').runner(exports);
