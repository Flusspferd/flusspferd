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

var Util = exports;

/**
 *  util.Range(form, to[, increment=1]) -> Iterator
 *  - from (Number): start of the range
 *  - end (Number): end of the range (non-inclusive)
 *  - increment (Number): increment value by this amount each loop.
 *
 *  Create an iterable object returning all objects in a range, starting at
 *  "from", up to (not including) "to", incrementing by "by" in each step.
 *
 *  ##### Example #
 *
 *      for (i in require('util').Range(2,10)) {
 *        print(i); // Prints 2 through 9
 *      }
 **/
Util.Range = function Range(from,to, by) {
  var i = from;
  by = by || 1;
  var r;
  function RangeInstance() {
    while (i < to) {
      // Store properties too.
      r[i] = i;
      yield i;
      i += by;
    }
  };
  r = new RangeInstance();

  return r;
}

