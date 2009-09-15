#!/usr/bin/env flusspferd

var getopt = require('getopt');
var io = require('io');

function print(x) {
  require('system').stdout.print(x);
}

function file_content(filename) {
  var f = new io.File();
  f.open(filename);
  var ret = f.readWhole();
  f.close();
  return ret;
}

var variables = new Array();

var option_spec = {
  "define": {
    "alias": "D",
    "argument": "required",
    "argument_type": "var_def",
    "doc": "Define a Variable (e.g. --define a=b).",
    "callback" : function(name, argument) {
      var v = argument.split(/=/);
      variables.push({ "name":"@"+v[0]+"@", "value":v[1] });
    }
  },
  "file-var": {
    "alias": "f",
    "argument": "required",
    "argument_type": "file_var_def",
    "doc": "Define a Variable containing the content of a file (e.g. --file-var a=file)",
    "callback" : function(name, argument) {
      var v = argument.split(/=/);
      if(v.length != 2) {
        throw "expected `var=filename', got `" + argument + "'";
      }
      variables.push({ "name":"@"+v[0]+"@", "value":file_content(v[1]) });
    }
  },
  "help": {
    "alias": "h",
    "doc": "Displays this message.",
    "callback" : function() {
      print("usage: replace-var [OPTIONS] [files ...]\n");
      print(getopt.getopt_help(option_spec));
      print("\n  @variable@ is replaced by the variable value.");
      quit();
    }
  }
};

var options = getopt.getopt(option_spec);

function replace_var(line) {
  for each(var v in variables) {
    line = line.replace(v["name"], v["value"]);
  }
  return line;
}

options["_"].shift();
for each(var filename in options["_"]) {
  var f = new io.File();
  f.open(filename);
  var line;
  while( (line = f.readLine()) ) {
    line = line.substring(0, line.length - 1); // remove trailing newline
    print(replace_var(line));
  }
  f.close();
}
