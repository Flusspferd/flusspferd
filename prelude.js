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

Array.from = function from(iterable) {
  if (!iterable)
    return [];
  if (iterable.toArray)
    return iterable.toArray();
  return Array.slice(iterable, 0);
};
Object.defineProperty(Array, 'from', {enumerable: false});

function Range(from,to, by) {
  var i = from;
  by = by || 1;
  var r;
  function RangeInstance() {
    while (i < to) {
      r[i] = i;
      yield i;
      i += by;
    }
  };
  r = new RangeInstance();
  r.__iterator__ = function() { return r };

  return r;
}

String.prototype.toArray = function toArray() {
  return this.split(/\s+/);
};
Object.defineProperty(String.prototype, 'toArray', {enumerable: false});

Function.prototype.bind = function bind(obj) {
  var fun = this;
  return function bound_func() {
      return fun.apply(obj, arguments);
    }
};
Object.defineProperty(Function.prototype, 'bind', {enumerable: false});

Object.defineProperty(Function, 'bind', 
  { enumerable: false, 
    value: function bind(obj, name) {
      var fun = obj[name];
      if (!fun || !(fun instanceof Function))
        throw new Error("Object has no function '" + name + "'");
      return fun.bind(obj);
    }
  });

(function (g) {
  var i = new Importer;

  i.load('IO');

  g.print = Function.bind(i.IO.stdout, 'print');
  g.readline = Function.bind(i.IO.stdin, 'readLine');

  g.IO = i.IO;

  i.load('XML');

  g.XML = i.XML;

  g.$importer = i;
})(this);


true;
