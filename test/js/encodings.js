const binary = require('binary');
const encodings = require('encodings');
const asserts = require('test').asserts;

// Test data.
const e_accute_utf8 = binary.ByteString([0xC3,0xA9]);
const partial_utf8 = binary.ByteString([0x,68,0xC3]);
const invalid_utf8 = binary.ByteString([0x,68,0xC3,0x6C]);

const e_accute_utf16 = binary.ByteString([0x0,0xe9]);
const partial_utf16 = binary.ByteString([0x,68,0xC3]);
const invalid_utf16 = binary.ByteString([0x,68,0xC3,0x6C]);

const ligature_oe_iso8859_15 = binary.ByteString([0xBD]);

if (!this.exports) this.exports = {};

exports.test_convertSameEnc = function() {
  var blob = encodings.convert("utf-8", "utf-8", e_accute_utf8);

  asserts.ok(blob !== e_accute_utf8, "different blob returned");
  asserts.same(blob.toArray(), [195,169]);
  
}

exports.test_convertToString = function() {
  var str = encodings.convertToString("utf-8", e_accute_utf8);

  asserts.same(str, "\xE9")
}


exports.test_convertToStringUTF16 = function() {
  var str = encodings.convertToString("utf-16", e_accute_utf16);

  asserts.same(str, "\xE9") // é
}

exports.test_convertToStringISO8859_15 = function() {
  var str = encodings.convertToString("iso-8859-15", ligature_oe_iso8859_15);

  asserts.same(str, "\u0153"); //œ
}

exports.test_convert = function() {
  var blob = encodings.convert("utf-8", "UTF-16", e_accute_utf8);

  asserts.same(blob.toArray(), [254,255,0,233]);
}

// if (module.isMain)
  require('test').runner(exports);
