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

#ifndef FLUSSPFERD_IO_FILE_CLASS_HPP
#define FLUSSPFERD_IO_FILE_CLASS_HPP

#include "stream.hpp"

namespace flusspferd { namespace io {

FLUSSPFERD_CLASS_DESCRIPTION(
  file,
  (base, stream)
  (full_name, "IO.File")
  (constructor_name, "File")
  (constructor_arity, 1)
  (methods,
    ("open", bind, open)
    ("close", bind, close))
  (constructor_methods,
    ("create", bind_static, create)
    ("exists", bind_static, exists)))
{
public:
  file(object const &, call_context &);
  file(object const &, char const* name, value mode);
  ~file();

public: // javascript methods
  void open(char const *name, value options);
  void close();

public: // constructor methods
  static void create(char const *name, boost::optional<int> mode);
  static bool exists(char const *name);

private:
  class impl;
  boost::scoped_ptr<impl> p;
};

}}

#endif
