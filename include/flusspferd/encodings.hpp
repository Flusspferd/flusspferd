// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
The MIT License

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
#ifndef FLUSSPFERD_ENCODINGS_HPP
#define FLUSSPFERD_ENCODINGS_HPP

#include "detail/api.hpp"
#include "string.hpp"
#include "binary.hpp"
#include "class_description.hpp"
#include <string>

namespace flusspferd {

FLUSSPFERD_API void load_encodings_module(object container);

namespace encodings {
  FLUSSPFERD_API string convert_to_string(std::string const &enc, binary &source);
  FLUSSPFERD_API object convert_from_string(
    std::string const &enc, flusspferd::string const &source);
  FLUSSPFERD_API object convert(
    std::string const &from_enc, std::string const &to_enc, binary &source);

  FLUSSPFERD_CLASS_DESCRIPTION(
    transcoder,
    (full_name, "encodings.Transcoder")
    (constructor_name, "Transcoder")
    (constructor_arity, 2)
    (methods,
      ("push", bind, push)
      ("close", bind, close)
      ("pushAccumulate", bind, push_accumulate)))
  {
  public:
    transcoder(object const &, std::string const &from, std::string const &to);
    transcoder(object const &, call_context &);

    ~transcoder();

    binary &push(binary &input, boost::optional<byte_array&> const &output);
    void push_accumulate(binary &input);
    binary &close(boost::optional<byte_array&> const &output);

  private:
    void init(std::string const &from, std::string const &to);
    void do_push(binary &input, binary::vector_type &output);
    binary &get_output_binary(boost::optional<byte_array&> const &output);
    void append_accumulator(binary &output);

    void trace(tracer &trc);

  private:
    class impl;
    boost::scoped_ptr<impl> p;
  };
}

}

#endif

