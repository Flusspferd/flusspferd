"use strict";

// TODO: Convert these to proper tests

const binary = require('binary');
const encodings = require('encodings');

const e_accute_utf8 = binary.ByteString([0xC3,0xA9]);
const partial_utf8 = binary.ByteString([0x,68,0xC3]);
const invalid_utf8 = binary.ByteString([0x,68,0xC3,0x6C]);

const e_accute_utf16 = binary.ByteString([0x0,0xe9,0x0]);
const partial_utf16 = binary.ByteString([0x,68,0xC3]);
const invalid_utf16 = binary.ByteString([0x,68,0xC3,0x6C]);

const ligature_oe_iso8859_15 = binary.ByteString([0xBD]);

//print(encodings.convertToString("utf-8", test1));

function test_convertSameEnc() {
  var blob = encodings.convert("utf-8", "utf-8", e_accute_utf8);

  print(blob !== e_accute_utf8);
  print(blob.toSource());
  
}

function test_convertToString() {
  var str = encodings.convertToString("utf-8", e_accute_utf8);

  print(str.toSource());
  print(typeof str == "string");
  print(str.length == 1);
  print (str == "\xE9");
  
}
function test_convertToStringUTF16() {
  var str = encodings.convertToString("utf-16", e_accute_utf16);

  print(str.toSource());
  print(typeof str == "string");
  print(str.length == 1);
  print (str == "\xE9"); // é
  
}

function test_convertToStringISO8859_15() {
  var str = encodings.convertToString("iso-8859-15", ligature_oe_iso8859_15);

  print(str.toSource());
  print(typeof str == "string");
  print(str.length == 1);
  print (str == "\u0153"); //œ
}

function test_convert() {
  var blob = encodings.convert("utf-8", "UTF-16", e_accute_utf8);

  print(blob !== e_accute_utf8);
  print(blob.toSource());
  
}

//test_convertSameEnc();
//test_convertToString();
//test_convertToStringUTF16();
test_convertToStringISO8859_15();
//test_convert();
