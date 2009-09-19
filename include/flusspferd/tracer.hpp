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

#ifndef FLUSSPFERD_TRACER_HPP
#define FLUSSPFERD_TRACER_HPP

#include "value.hpp"
#include <boost/scoped_ptr.hpp>
#include <string>

namespace flusspferd {

class context;
class value;

/**
 * Garbage collection tracer.
 *
 * @see native_object_base::trace
 *
 * @ingroup gc
 */
class tracer {
public:
#ifndef IN_DOXYGEN
  void trace_gcptr(char const *name, void *gcptr);

  void trace_gctr(std::string const &name, void *gcptr) {
    trace_gcptr(name.c_str(), gcptr);
  }
#endif

  void operator()(char const *name, value val) {
    trace_gcptr(name, val.get_gcptr());
  }

  void operator()(std::string const &name, value val) {
    trace_gcptr(name.c_str(), val.get_gcptr());
  }

public: //internal
  tracer(void *opaque);
  ~tracer();

private:
  class impl;
  boost::scoped_ptr<impl> p;
};

}

#endif
