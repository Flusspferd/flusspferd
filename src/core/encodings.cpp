// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#include <iconv.h>
#include <errno.h>
#include <sstream>
#include "flusspferd/binary.hpp"
#include "flusspferd/encodings.hpp"
#include "flusspferd/create.hpp"



using namespace flusspferd;

// If no direct conversion is possible, do it via utf8. Helper method
object convert_via_utf8(const char* toEnc, const char* fromEnc, binary const &source);

void flusspferd::load_encodings_module(object container) {
  object exports = container.get_property_object("exports");

  // Load the binary module
  global().call("require", "binary");

  create_native_function(
    exports,
    "convertToString", &encodings::convert_to_string);

  create_native_function(
    exports,
    "convertFromString", &encodings::convert_from_string);

  create_native_function( exports, "convert", &encodings::convert);
}

string encodings::convert_to_string(const char *, binary &) {
  return string();
}


object encodings::convert_from_string(const char*, string const &) {


  size_t n = 0;
  return create_native_object<byte_string>(object(), (unsigned char*)"", n);
}

object encodings::convert(const char*fromEnc, const char *toEnc,
    binary &source_binary)
{
  binary::vector_type const &source = source_binary.get_const_data();


  if ( strcmp(fromEnc, toEnc) == 0) {
    // Encodings are the same, just return a copy of the binary
    return create_native_object<byte_string>(
      object(),
      &source[0],
      source.size()
    );
  }

  iconv_t conv = iconv_open(toEnc, fromEnc);

  if (conv == (iconv_t)(-1)) {
    std::stringstream ss;
    ss << "Unable to convert from \"" << fromEnc
       << "\" to \"" << toEnc << "\"";
    throw exception(ss.str().c_str());
  }

  binary::vector_type outbuf;
  outbuf.reserve(source.size());

  size_t out_left = outbuf.size(),
         in_left  = source.size();

  // I'm sure this can be done nicer
  const unsigned char *inbytes  = &source[0],
                      *outbytes = &outbuf[0];

  char **inptr  = (char**)&inbytes,
       **outptr = (char**)&outbytes;

  while (in_left) {
    size_t n = iconv(conv, inptr, &in_left, outptr, &out_left);

    if (n == (size_t)(-1)) {
      switch (errno) {
        case E2BIG: // Not enough space in output
          break;
        case EILSEQ:
          // An invalid multibyte sequence has been encountered in the input.
        case EINVAL:
          // An incomplete multibyte sequence has been encountered in the input.
          //
          // Since we have provided the entire input, both these cases are the same.
          break;
      }
    }
  }
  return create_native_object<byte_string>(object(), &outbuf[0], outbuf.size());
}

object convert_via_utf8(const char* toEnc, const char* fromEnc, binary const &) {
  iconv_t to_utf   = iconv_open("utf-8", fromEnc),
          from_utf = iconv_open(toEnc, "utf-8");

  if (to_utf == (iconv_t)(-1) || from_utf == (iconv_t)(-1)) {

    if (to_utf)
      iconv_close(to_utf);
    if (from_utf)
      iconv_close(from_utf);

    std::stringstream ss;
    ss << "Unable to convert from \"" << fromEnc
       << "\" to \"" << toEnc << "\"";
    throw exception(ss.str().c_str());
  }
  return object();
}
