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

#ifndef FLUSSPFERD_IO_BINARY_STREAM_HPP
#define FLUSSPFERD_IO_BINARY_STREAM_HPP

#include "stream.hpp"
#include "../binary.hpp"

namespace flusspferd { namespace io {

FLUSSPFERD_CLASS_DESCRIPTION(
  binary_stream,
  (base, stream)
  (full_name, "IO.BinaryStream")
  (constructor_name, "BinaryStream")
  (constructor_arity, 1)
  (methods,
    ("getBinary", bind, get_binary)))
{
public:
  binary_stream(object const &, call_context &);
  ~binary_stream();

protected:
  void trace(tracer &);

public: // javascript methods
  binary &get_binary();

private:
  class impl;
  boost::scoped_ptr<impl> p;
};

}}

#endif
