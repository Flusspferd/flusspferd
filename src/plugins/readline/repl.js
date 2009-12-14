#!/usr/bin/env flusspferd
// a Javascript REPL in Javascript

const Readline = require('readline');

var str;
while( (str = Readline.readline('# ')) ) {
  print(eval(str).toSource());
  gc();
}
