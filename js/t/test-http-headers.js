i = new Importer();
i.context = this;
i.paths.push('js/src');

i.load('TestHarness');
i.load('HTTP.Headers');

t = new TestHarness();

t.test_simple = function() {
  this.expect(1);
  const hdr_str = "Content-type: text/html; charset=utf8\r\n";
  this.same(
    HTTP.Headers.parse(hdr_str),
    new HTTP.Headers({"Content-Type": "text/html; charset=utf8"})
  );
}

t.test_multiline = function() {
  this.expect(1);
  const hdr_str = "Set-Cookie: value=foo\r\n   bar;path=/\r\n";

  got = HTTP.Headers.parse(hdr_str);
  this.same(
    got,
    new HTTP.Headers({"Set-Cookie": "value=foo bar;path=/"}),
    "multi-line header parsed okay"
  );
}

t.go();
