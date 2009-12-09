#!/usr/bin/env flusspferd

// cd src/plugins/curl && git checkout curl.pdoc && flusspferd gen-doc.js >> curl.pdoc && cd ../../.. && make doc

const io = require('io');

function print(x) {
  require('system').stdout.print(x);
}

function trim(x) {
  var t = x.replace(/\s*$/,'').replace(/^\s*/,'');
  return t.replace(/^%/,''); // % is a hack to stop triming if indentation is important
}

const sys = require('system');
const file = sys.args.length > 1 ? sys.args[1] : 'curl.cpp';

var fh = new io.File();
fh.open(file);

var in_doc = false;
var in_header = false;
var header = new Array;
var line;
while( (line = fh.readLine()) ) {
  if(line.indexOf('BEGIN DOC{') != -1) {
    print('\n/**');
    in_doc = true;
  }
  else if(line.indexOf('}END DOC') != -1) {
    print(' **/');
    in_doc = false;
  }
  else if(in_header) {
    if(line.match(/}HEADER/)) {
      for each(var i in header) {
        print(' * ' + i);
      }
      print(' * ');
      in_header = false;
    }
    header.push(trim(line));
  }
  else if(in_doc) {
    var m;
    if( (m = line.match(/ptr_map_insert< (.*?)_option<(.*?)> >\(map\)\("(.*?)"\)/)) ) {
      var out = ' * + ' + m[3] + ', ' + m[1];
      var n;
      if( (n = line.match(/\/\/ (.*)/)) ) {
        out += '. ' + n[1];
      }
      out += ' ([' + m[2] +
        '](http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#' +
        m[2].replace(/_/g,'') + '))';
      print(out);
    }
    else if( (m = line.match(/HEADER{/))) {
      in_header = true;
    }
    else if( (m = line.match(/^\s*\/\/ (.*)/))) {
      if(m[1].indexOf('TODO') == -1) {
        print(' * ###### ' + m[1]);
      }
    }
  }
}
fh.close();
