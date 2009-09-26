// vim:ts=2:sw=2:expandtab:autoindent:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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

const asserts = require('test').asserts,
      HTTP = require('http/headers');

if (!this.exports) this.exports = {};

exports.test_simple = function() {
  const hdr_str = "Content-type: text/html; charset=utf8\r\n";
  asserts.same(
    HTTP.Headers.parse(hdr_str),
    new HTTP.Headers({"Content-Type": "text/html; charset=utf8"})
  );
}

exports.test_multiline = function() {
  const hdr_str = "Set-Cookie: value=foo\r\n   bar;path=/\r\n";

  var got = HTTP.Headers.parse(hdr_str);
  asserts.same(
    got,
    new HTTP.Headers({"Set-Cookie": "value=foo bar;path=/"}),
    "multi-line header parsed okay"
  );
}

require('test').runner(exports);
