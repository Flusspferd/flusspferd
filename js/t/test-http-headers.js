// vim:ts=2:sw=2:expandtab:autoindent:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

i = new requireer();
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
