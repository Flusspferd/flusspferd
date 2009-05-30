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

#include "flusspferd/binary.hpp"
#include "flusspferd/encodings.hpp"
#include "flusspferd/create.hpp"

using namespace flusspferd;

void flusspferd::load_encodings_module(object container) {
  object exports = container.get_property("exports").to_object();

  // Load the binary module
  global().call("require", "binary");

  create_native_function(
    exports,
    "convertToString", &encodings::convert_to_string);

  create_native_function(
    exports,
    "convertFromString", &encodings::convert_from_string);

  //create_native_function( exports, "convert", &encodings::convert);
}

string encodings::convert_to_string(const char *, binary const &) {
  return string();
}


object encodings::convert_from_string(const char* enc, string const &source) {
  size_t n = 0;
  return create_native_object<byte_string>(object(), (unsigned char*)"", n);
}

object encodings::convert(const char*fromEnc, const char *toEnc,
    binary const &source) {

  size_t n = 0;
  return create_native_object<byte_string>(object(), (unsigned char*)"", n);
}
