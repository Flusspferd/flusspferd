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

#include "flusspferd/io/binary_stream.hpp"
#include "flusspferd/tracer.hpp"
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/concepts.hpp>
#include <cstdlib>

using namespace flusspferd;
using namespace flusspferd::io;

namespace {

struct binary_device : 
  boost::iostreams::device<
    boost::iostreams::bidirectional_seekable>
{
  explicit binary_device(binary &binary_)
    : v(binary_.get_data()), pos_read(0), pos_write(0), read_only(true)
  {}

  std::streamsize read(char *s, std::streamsize n);
  std::streamsize write(char const *s, std::streamsize n);
  std::streampos seek(
    boost::iostreams::stream_offset off,
    std::ios::seekdir way,
    std::ios::openmode which);

  std::vector<unsigned char> &v;
  std::size_t pos_read;
  std::size_t pos_write;

  bool read_only;
};

}

class binary_stream::impl {
public:
  impl(binary &binary_)
    : binary_(binary_), buf(binary_device(binary_), 0)
  {}

  binary &binary_;
  boost::iostreams::stream_buffer<binary_device> buf;
};

static binary &get_arg(call_context &x) {
  if (!x.arg[0].is_object() || x.arg[0].is_null())
    throw exception("Could not create BlobStream without Blob");
  object obj = x.arg[0].get_object();
  return flusspferd::get_native<binary>(obj);
}

binary_stream::binary_stream(object const &obj, call_context &x)
  : base_type(obj, (std::streambuf*)0), p(new impl(get_arg(x)))
{
  if (dynamic_cast<byte_array*>(&p->binary_)) {
    p->buf->read_only = false;
  }
  set_streambuf(&p->buf);
}

binary_stream::~binary_stream()
{}

void binary_stream::trace(tracer &trc) {
  trc("binary", p->binary_);
}

binary &binary_stream::get_binary() {
  return p->binary_;
}

std::streamsize binary_device::read(char *data, std::streamsize n) {
  if (pos_read >= v.size())
    return -1;
  std::size_t n_left = v.size() - pos_read;
  if (n < 0)
    n = 0;
  if (std::size_t(n) > n_left)
    n = n_left;
  std::memcpy(data, &v[pos_read], n);
  pos_read += n;
  return n;
}

std::streamsize binary_device::write(char const *data, std::streamsize n) {
  if (read_only)
    return -1;

  if (n < 0)
    n = 0;
  if (pos_write + n >= v.size())
    v.resize(pos_write + n);
  std::memcpy(&v[pos_write], data, n);
  pos_write += n;
  return n;
}

std::streampos binary_device::seek(
    boost::iostreams::stream_offset off,
    std::ios::seekdir way,
    std::ios::openmode which)
{
  std::size_t *p_pos;
  if (which == std::ios::in)
    p_pos = &pos_read;
  else
    p_pos = &pos_write;
  switch (way) {
  case std::ios_base::beg:
    *p_pos = off;
    break;
  case std::ios_base::cur:
    *p_pos += off;
    break;
  case std::ios_base::end:
    *p_pos = v.size() + off;
    break;
  default:
    assert(false && "strange stdlib behaviour. (_S_ios_seekdir_end)");
    break;
  }
  return *p_pos;
}
