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

#ifndef FLUSSPFERD_IO_STREAM_BASE_HPP
#define FLUSSPFERD_IO_STREAM_BASE_HPP

#include "../native_object_base.hpp"
#include "../class.hpp"
#include "../class_description.hpp"
#include "../string.hpp"
#include "../binary.hpp"
#include <streambuf>

namespace flusspferd { namespace io {

FLUSSPFERD_CLASS_DESCRIPTION(
  stream,
  (full_name, "IO.Stream")
  (constructor_name, "Stream")
  (constructible, false)
  (methods,
    ("readWhole", bind, read_whole)
    ("read", bind, read)
    ("readWholeBinary", bind, read_whole_binary)
    ("readBinary", bind, read_binary)
    ("write", bind, write)
    ("flush", bind, flush)
    ("print", bind, print)
    ("readLine", bind, read_line))
  (properties,
    ("fieldSeparator", variable, " ")
    ("recordSeparator", variable, "\n")
    ("autoFlush", variable, false)))
{
public:
  stream(object const &o, std::streambuf *b);
  ~stream();

  std::streambuf *streambuf();
  void set_streambuf(std::streambuf *buf);

public: // javascript methods
  string read_whole();
  string read(boost::optional<unsigned> max_size);

  object read_whole_binary(boost::optional<byte_array&> output);
  object read_binary(boost::optional<unsigned> max_size, boost::optional<byte_array&> output);

  void write(value const &);

  void flush();

  void print(call_context &);
  string read_line(value sep);

private:
  std::streambuf *streambuf_;
};

}}

#endif
