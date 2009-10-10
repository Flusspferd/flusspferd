const binary = require('binary');
const encodings = require('encodings');
const asserts = require('test').asserts;

// Test data.
const e_accute_utf8 = binary.ByteString([0xC3,0xA9]),
      partial_utf8 = binary.ByteString([0x,68,0xC3]),
      invalid_utf8 = binary.ByteString([0x,68,0xC3,0x6C]),

      e_accute_utf16 = binary.ByteString([0x0,0xe9]),
      partial_utf16 = binary.ByteString([0x,68,0xC3]),
      invalid_utf16 = binary.ByteString([0x,68,0xC3,0x6C]),

      ligature_oe_iso8859_15 = binary.ByteString([0xBD]),

      // Te Su To - thanks to miwagawa
      katakana_string = "\u30c6\u30b9\u30c8",
      katakana_shift_jis = binary.ByteString([
        0x83,0x65,
        0x83,0x58,
        0x83,0x67
      ]),

      // Shift to JIS X 0208-1983
      iso2022_FROM_ascii = [ 0x1b,0x24,0x42 ],
      // Shift to ASCII
      iso2022_TO_ascii   = [ 0x1b,0x28,0x42 ],

      katakana_iso2022 = binary.ByteString(
        iso2022_FROM_ascii.concat([
          0x25,0x46,
          0x25,0x39,
          0x25,0x48,
        ], iso2022_TO_ascii)
      );

if (!this.exports) this.exports = {};

exports.test_convertSameEnc = function() {
  var blob = encodings.convert("utf-8", "utf-8", e_accute_utf8);

  asserts.instanceOf(blob, binary.Binary);
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

  asserts.instanceOf(blob, binary.Binary);
  asserts.same(blob.toArray(), [254,255,0,233]);
}

exports.test_katakanaTranscoder_OneShot = function() {
  var c = new encodings.Transcoder("ISO-2022-JP", "Shift_JIS")

  asserts.instanceOf(c, encodings.Transcoder);

  asserts.same(
    c.pushAccumulate(katakana_iso2022),
    undefined,
    "transcoder.pushAccumulate(blob) returns undefined"
  );

  var blob = c.close();
  asserts.instanceOf(blob, binary.ByteString);

  asserts.same(
    blob.toArray(),
    katakana_shift_jis.toArray(),
    "Correct Shift_JIS output"
  );

  asserts.same(
    encodings.convertToString("Shift_JIS", blob),
    katakana_string,
    "Correct unicode string"
  );
}

exports.test_katakanaTranscoder_Chunks = function() {
  var c = new encodings.Transcoder("Shift_JIS", "ISO-2022-JP")
  var a = new binary.ByteArray();

  const iso2022 = katakana_iso2022.toArray();

  asserts.instanceOf(c, encodings.Transcoder, "Transcoder created");

  // Each 2 bytes of the Shift_JIS (when non-ascii) = 1 unicode character.

  // Send first two bytes
  asserts.same(
    c.push(katakana_shift_jis.substr(0,2), a),
    a
  );

  // Check we get the conversion seq + first code point
  asserts.same(
    a.toArray(),
    iso2022.slice(0,5), //[0x1b,0x24,0x42,0x25,0x46]
    "correct ISO-2022-JP opening"
  );

  // Rest of the data
  c.push(katakana_shift_jis.slice(2), a);

  // Test that the chars have been output, but not the final shift back to ascii
  asserts.same(
    a.toArray(),
    katakana_iso2022.substr(
      0, katakana_iso2022.length - iso2022_TO_ascii.length
    ).toArray(),
    "ISO-2022-JP->ascii not yet sent"
  );

  c.close(a);

  asserts.same(
    a.toArray(),
    katakana_iso2022.toArray(),
    "ISO-2022-JP->ascii added only after close"
  );
}

if (require.main === module)
  require('test').runner(exports);
