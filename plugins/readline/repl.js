#!/usr/bin/env flusspferd
// a Javascript REPL in Javascript

const Readline = require('readline');

Readline.usingHistory();

try {
  Readline.readHistory("/tmp/hist.txt");
}
catch(e) { }

var str;
while( (str = Readline.readline('# ')) ) {
  print(eval(str).toSource());
  Readline.addHistory(str);
  gc();
}

Readline.writeHistory("/tmp/hist.txt");
