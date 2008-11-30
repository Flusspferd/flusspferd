// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008 Aristid Breitkreuz, Ruediger Sonderfeld

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

#include "flusspferd/context.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/io/io.hpp"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

BOOST_AUTO_TEST_SUITE( io )

BOOST_AUTO_TEST_CASE( test_file_read ) {
  flusspferd::init::initialize();
  flusspferd::context co = flusspferd::context::create();
  flusspferd::current_context_scope scope(co);
  flusspferd::io::load_io();
  const char* js = "f = new IO.File('test/fixtures/file1'); f.readWhole()";
  flusspferd::value v;
  BOOST_CHECK_NO_THROW( v = co.evaluate(js, strlen(js), __FILE__, __LINE__) );
  BOOST_CHECK_EQUAL(v.to_string().c_str(), "foobar\nbaz\n");

  js = "f = new IO.File('test/fixtures/file1'); f.readWhole()";
  BOOST_CHECK_NO_THROW( v = co.evaluate(js, strlen(js), __FILE__, __LINE__) );
  BOOST_CHECK_EQUAL(v.to_string().c_str(), "foobar\nbaz\n");

  co.gc();
}


BOOST_AUTO_TEST_SUITE_END()

