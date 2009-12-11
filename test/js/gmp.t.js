const gmp = require('gmp');
const asserts = require('test').asserts;

exports.testInteger = function() {
  var j = gmp.Integer(10);
  asserts.ok(j.fitsInt());
  asserts.same(j.toInt(), 10);
  asserts.same(j.toDouble(), 10.0);
  asserts.same(j.toString(), '10');
  asserts.same(j.toString(2), '1010');
  asserts.ok(j.sgn() > 0);
  var z = gmp.Integer(1);
  asserts.ok(z.fitsInt());
  asserts.same(z.toInt(), 1);
  asserts.same(z.toString(), '1');
  asserts.same(z.toString(2), '1');
  var r = j.add(z);
  asserts.ok(r.fitsInt());
  asserts.same(r.toInt(), 11);
  asserts.same(r.toString(), '11');
  asserts.same(r.toString(2), '1011');
  z = gmp.Integer(-5);
  asserts.ok(z.fitsInt());
  asserts.same(z.toInt(), -5);
  asserts.same(z.toString(), '-5');
  asserts.same(z.toString(2), '-101');
  asserts.ok(z.sgn() < 0);
  r = j.add(z);
  asserts.ok(r.fitsInt());
  asserts.same(r.toInt(), 5);
  asserts.same(r.toString(), '5');
  asserts.same(r.toString(2), '101');
  r = z.abs();
  asserts.same(r.toInt(), 5);
  asserts.same(r.toString(), '5');
  asserts.same(r.toString(2), '101');
  asserts.same(r.sqrt().cmp(gmp.Integer(2)), 0);
  asserts.ok(r.cmp(j) < 0);
  asserts.ok(j.cmp(r) > 0);
  asserts.same(r.cmp(gmp.Integer(5)), 0);
  j = gmp.Integer("0xFFFF");
  asserts.ok(j.fitsInt());
  asserts.same(j.toInt(), 0xFFFF);
  asserts.same(j.toString(), '65535');
  j = gmp.Integer("0xFFFFFFFFFFFF");
  asserts.ok(!j.fitsInt());
  asserts.ok(j.cmp(r) > 0);

  asserts.throwsOk(function() { gmp.Integer(true); }, TypeError);
  asserts.throwsOk(function() { gmp.Integer("invalid"); });
  asserts.throwsOk(function() { gmp.Integer("invalid", 10); });
  asserts.throwsOk(function() { gmp.Integer(1,2,3,4,5); });
};

exports.testRational = function() {
  var q = gmp.Rational(10,1);
  asserts.same(q.toDouble(), 10);
  asserts.same(q.toString(), '10');
  asserts.same(q.toString(2), '1010');
  q.denominator = gmp.Integer(3);
  asserts.same(q.denominator.toInt(), 3);
  asserts.same(q.toString(), '10/3');
  q.numerator = gmp.Integer(3);
  asserts.same(q.numerator.toInt(), 3);
  asserts.same(q.toString(), '3/3');
  q.canonicalize();
  asserts.same(q.toString(), '1');
  var p = gmp.Rational(2,1);
  asserts.ok(q.cmp(p) < 0);
  asserts.ok(p.cmp(q) > 0);
  p.numerator = gmp.Integer(1);
  asserts.same(p.cmp(q), q.cmp(p));
  asserts.same(p.cmp(q), 0);
  p.numerator = gmp.Integer(2);
  var pp = gmp.Rational("2");
  asserts.same(p.cmp(pp), 0);

  asserts.throwsOk(function() { gmp.Rational(true); }, TypeError);
  asserts.throwsOk(function() { gmp.Rational("invalid"); });
  asserts.throwsOk(function() { gmp.Rational("invalid", 10); });
  asserts.throwsOk(function() { gmp.Rational(1,2,3,4,5); });
};

exports.testFloat = function() {
  var f = gmp.Float(10);
  asserts.ok(f.fitsInt());
  asserts.same(f.toInt(), 10);
  asserts.same(f.toString(), '10.0');
  asserts.same(f.toString(2), '1010.0');
  asserts.ok(f.fitsInt());
  var t = gmp.Float(2);
  asserts.ok(t.fitsInt());
  asserts.same(t.toInt(), 2);
  var s = t.sqrt();
  asserts.ok(s.cmp(t) < 0);
  asserts.ok(s.cmp(1) > 0);
  asserts.same(s.floor().cmp(1), 0);
  asserts.same(s.ceil().cmp(2), 0);

  asserts.throwsOk(function() { gmp.Float(true); }, TypeError);
  asserts.throwsOk(function() { gmp.Float("invalid"); });
  asserts.throwsOk(function() { gmp.Float("invalid", 10); });
  asserts.throwsOk(function() { gmp.Float(1,2,3,4,5); });
};

if (require.main === module)
  require('test').runner(exports);
