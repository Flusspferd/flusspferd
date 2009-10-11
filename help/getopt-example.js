// -*- mode: js2; coding: utf-8; -*- vim:ts=2:sw=2:expandtab:autoindent:enc=utf-8:
/*
The MIT License

Copyright (c) 2009 Flusspferd contributors (see "CONTRIBUTORS" or
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
var spec = {
  "help" : {
    "alias" : "h",
    "doc" : "Displays this message.",
    "callback" : function() {
      print("usage: getopt-example.js [OPTIONS] ...");
      print(getopt.getopt_help(spec));
      quit();
    }
  },
  "version" : {
    "alias" : "v",
    "doc" : "Show version number.",
    "callback" : function() {
      print("Version 1.0");
      quit();
    }
  },
  "foo" : {
     "alias" : ["f", "F"],
     "argument" : "required",
     "argument_type" : "file",
     "doc" : "Foo!",
     "callback" : function(name, argument) {
       print("option '" + name + "' argument '" + argument + "'");
     }
  },
  "bar" : {
     "alias" : ["baz", "b" ],
     "argument" : "optional",
     "doc" : "Bar is the way to go.",
     "callback" : function(name, argument) {
       if(argument) {
         print("option '" + name + "' called with argument '" + argument + "'");
       }
       else {
         print("option '" + name + "' called without argument");
       }
     }
  }
};

var getopt = require('getopt');
var result = getopt.getopt(spec);

print(result["_"]);
