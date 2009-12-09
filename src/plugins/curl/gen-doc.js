#!/usr/bin/env flusspferd

const io = require('io');

function print(x) {
  require('system').stdout.print(x);
}

const file = 'curl.cpp';

var fh = new io.File();
fh.open(file);

var in_doc = false;
var line;
while( (line = fh.readLine()) ) {
  if(line.indexOf('BEGIN DOC{') != -1) {
    in_doc = true;
  }
  else if(line.indexOf('}END DOC') != -1) {
    in_doc = false;
  }
  else if(in_doc) {
    var m;
    if( (m = line.match(/ptr_map_insert< (.*?)_option<(.*?)> >\(map\)\("(.*?)"\)/)) ) {
      var out = ' * ' + m[2] + ' -> ' + m[3] + ' (' + m[1]+ ')';
      if( (m = line.match(/\/\/ (.*)/)) ) {
        out += ' ' + m[1];
      }
      print(out);
    }
    else if( (m = line.match(/^\s*\/\/ (.*)/))) {
      if(m[1].indexOf('TODO') == -1) {
        print(' * ###### ' + m[1]);
      }
    }
  }
}
fh.close();
