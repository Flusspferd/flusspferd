"use strict";

// TODO: Convert these to proper tests

const binary = require('binary');
const encodings = require('encodings');

const e_accute = binary.ByteString([0xC3,0xA9]);
const partial_utf8 = binary.ByteString([0x,68,0xC3]);
const invalid_utf8 = binary.ByteString([0x,68,0xC3,0x6C]);

//print(encodings.convertToString("utf-8", test1));

function test_convertSameEnc() {
  var blob = encodings.convert("utf-8", "utf-8", e_accute);

  print(blob !== e_accute);
  print(blob.toSource());
  
}

function test_convertToString() {
  var str = encodings.convertToString("utf-8", e_accute);

  print(str.toSource());
  print(typeof str == "string");
  print(str.length == 1);
  print (str == "\xE9");
  
}

function test_convert() {
  var blob = encodings.convert("utf-8", "UTF-16", e_accute);

  print(blob !== e_accute);
  print(blob.toSource());
  
}

//test_convertSameEnc();
test_convertToString();
//test_convert();
