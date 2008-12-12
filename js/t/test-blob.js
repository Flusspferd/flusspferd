i = new Importer();
i.context = this;
i.paths.push('js/src');

i.load('TestHarness');

t = new TestHarness();

t.test_simple = function() {
  this.expect(2);

  b = new Blob(0);
  this.ok(b instanceof Blob, "instanceof");

  this.same(b.length, 0, "length 0");
}

t.go();
