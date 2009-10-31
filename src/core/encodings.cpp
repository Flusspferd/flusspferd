// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

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
#include "flusspferd/create/native_object.hpp"
#include <iconv.h>
#include <errno.h>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/ref.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>

using namespace boost;
using namespace flusspferd;
using namespace fusion;

void flusspferd::load_encodings_module(object container) {
  object exports = container.get_property_object("exports");

  // Load the binary module
  container.call("require", "binary");

  create_native_function(
    exports,
    "convertToString", &encodings::convert_to_string);

  create_native_function(
    exports,
    "convertFromString", &encodings::convert_from_string);

  create_native_function(
    exports,
    "convert", &encodings::convert);

  load_class<encodings::transcoder>(exports);
}

// the UTF-16 bom is codepoint U+feff
static char16_t const bom_le = *(char16_t*)"\xff\xfe";
static char16_t const bom_native = 0xfeff;

static char const * const native_charset = bom_le == bom_native
                                         ? "utf-16le" : "utf-16be";


// JAVASCRIPT METHODS

flusspferd::string
encodings::convert_to_string(std::string const &enc_, binary &source_binary) {
  transcoder &trans =
    create<transcoder>(
      vector2<std::string const&, std::string const&>(enc_, native_charset));
  root_object root_obj(trans);

  trans.push_accumulate(source_binary);

  binary &out = trans.close(boost::none);

  return flusspferd::string(
    reinterpret_cast<char16_t const *>(&out.get_data()[0]),
    out.get_length() / sizeof(char16_t));
}

object encodings::convert_from_string(std::string const &enc, string const &str)
{
  transcoder &trans =
    create<transcoder>(
      vector2<std::string const&, std::string const&>(native_charset, enc));
  root_object root_obj(trans);

  binary &source_binary = create<byte_string>(
    vector2<binary::element_type const *, std::size_t>(
      reinterpret_cast<binary::element_type const*>(str.data()),
      str.size() * sizeof(char16_t)));
  root_object root_obj2(source_binary);

  trans.push_accumulate(source_binary);

  return trans.close(boost::none);
}

object encodings::convert(
  std::string const &from_, std::string const &to_, binary &source_binary)
{
  transcoder &trans = create<transcoder>(make_vector(cref(from_), cref(to_)));
  root_object root_obj(trans);

  trans.push_accumulate(source_binary);

  return trans.close(boost::none);
}

// TRANSCODER

class encodings::transcoder::impl {
public:
  impl()
  : conv(iconv_t(-1))
  {}

  ~impl() {
    if (conv != iconv_t(-1))
      iconv_close(conv);
  }

  binary::vector_type accumulator;
  binary::vector_type multibyte_part;

  iconv_t conv;
};

void encodings::transcoder::trace(tracer &) {
}

encodings::transcoder::transcoder(object const &obj, call_context &x)
: base_type(obj)
{
  init(x.arg[0].to_std_string(), x.arg[1].to_std_string());
}

encodings::transcoder::transcoder(
  object const &obj, std::string const &from, std::string const &to)
: base_type(obj)
{
  init(from, to);
}

encodings::transcoder::~transcoder() {
}

void encodings::transcoder::init(std::string const &from, std::string const &to)
{
  boost::scoped_ptr<impl> p(new impl);

  define_property(
    "sourceCharset",
    flusspferd::value(from),
    property_attributes(read_only_property | permanent_property));

  define_property(
    "destinationCharset",
    flusspferd::value(to),
    property_attributes(read_only_property | permanent_property));

  p->conv = iconv_open(to.c_str(), from.c_str());

  if (p->conv == iconv_t(-1)) {
    std::ostringstream message;
    message << "Could not create Transcoder (iconv) "
            << "from charset \"" << from << "\" "
            << "to charset \"" << to << "\"";
    throw exception(message.str());
  }

  this->p.swap(p);
}

binary &encodings::transcoder::push(
  binary &input, boost::optional<byte_array&> const &output_)
{
  binary &output = get_output_binary(output_);

  root_object root_obj(output);

  append_accumulator(output);
  do_push(input, output.get_data());

  return output;
}

void encodings::transcoder::push_accumulate(binary &input) {
  do_push(input, p->accumulator);
}

binary &encodings::transcoder::close(
  boost::optional<byte_array&> const &output_)
{
  if (!p->multibyte_part.empty())
    throw exception("Invalid multibyte sequence at the end of input");

  binary &output = get_output_binary(output_);

  root_object root_obj(output);

  append_accumulator(output);

  if (p->conv != iconv_t(-1)) {
    binary::vector_type &out_v = output.get_data();
    std::size_t start = out_v.size();
    // 32 bytes should suffice for the initial state shift
    std::size_t outlen = 32;
    out_v.resize(out_v.size() + outlen);
    char *outbuf = reinterpret_cast<char*>(&out_v[start]);
    if (iconv(p->conv, 0, 0, &outbuf, &outlen) == std::size_t(-1))
      throw exception("Adding closing character sequence failed");
    out_v.resize(out_v.size() - outlen);

    if (iconv_close(p->conv) == -1)
      throw exception("Closing character set conversion descriptor failed");

    p->conv = iconv_t(-1);
  }

  return output;
}

binary &encodings::transcoder::get_output_binary(
  boost::optional<byte_array&> const &output_)
{
  return
    output_
    ? static_cast<binary&>(output_.get())
    : static_cast<binary&>(
        create<byte_string>(vector2<binary::element_type*, std::size_t>(0,0)));
}


void encodings::transcoder::do_push(binary &input, binary::vector_type &out_v) {
  binary::vector_type &in_v =
    p->multibyte_part.empty() ? input.get_data() : p->multibyte_part;

  if (!p->multibyte_part.empty())
    in_v.insert(in_v.end(), input.get_data().begin(), input.get_data().end());

  // A rough guess how much space might be needed for the new characters.
  std::size_t out_estimate = in_v.size() + in_v.size()/16 + 32;

  std::size_t out_start = out_v.size();

  for (;;) {
    out_v.resize(out_v.size() + out_estimate);

#ifdef ICONV_ACCEPTS_NONCONST_INPUT
    char *inbuf;
#else
    char const *inbuf;
#endif

    inbuf = reinterpret_cast<char *>(&in_v[0]);

    char *outbuf = reinterpret_cast<char*>(&out_v[out_start]);

    std::size_t inbytesleft = in_v.size();
    std::size_t outbytesleft = out_estimate;

    std::size_t n_chars = iconv(
      p->conv,
      &inbuf, &inbytesleft,
      &outbuf, &outbytesleft);

    if (n_chars == std::size_t(-1)) {
      switch (errno) {
      case EILSEQ:
        throw exception("Invalid multi-byte sequence in input");

      case E2BIG:
        out_v.resize(out_v.size() - out_estimate);
        out_estimate *= 2;
        break;

      case EINVAL:
        p->multibyte_part.assign(outbuf, outbuf + outbytesleft);
        out_v.resize(out_v.size() - outbytesleft);
        return;

      default:
        throw exception("Unknown error in character conversion");
      }
    } else {
      out_v.resize(out_v.size() - outbytesleft);
      return;
    }
  }
}

void encodings::transcoder::append_accumulator(binary &output) {
  binary::vector_type &out_v = output.get_data();

  if (!p->accumulator.empty()) {
    if (!out_v.empty()) {
      out_v.insert(out_v.end(), p->accumulator.begin(), p->accumulator.end());
      binary::vector_type().swap(p->accumulator);
    } else {
      out_v.swap(p->accumulator);
    }
  }
}
