// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#ifndef FLUSSPFERD_IMPORTER_HPP
#define FLUSSPFERD_IMPORTER_HPP

#include "native_object_base.hpp"
#include "class.hpp"
#include <boost/scoped_ptr.hpp>

namespace flusspferd { 

class importer : public native_object_base {

public:
  // The type of function which we look for in loaded .so modules
  // extern "C" value flusspferd_load(object container);
  typedef value (*flusspferd_load_t)(object container);

  struct class_info : flusspferd::class_info {
    typedef boost::mpl::bool_<true> constructible;
    static char const* constructor_name() { return "Importer"; }
    static object create_prototype();
    static void augment_constructor(object &);
  };

  importer(object const &obj, call_context &x);
  ~importer();

protected:
  void trace(tracer &);

  static std::string process_name(string const &name, bool for_script = false);

private: // JS methods
  value load(string const &name, bool binary_only); 

private:
  class impl;
  boost::scoped_ptr<impl> p;
};

}

#endif
