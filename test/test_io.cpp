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

#include "flusspferd/object.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/security.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/io/io.hpp"
#include "test_environment.hpp"
#include <iostream>
#include <string>


BOOST_FIXTURE_TEST_SUITE( io, context_fixture )
#ifdef FLUSSPFERD_HAVE_IO

BOOST_AUTO_TEST_CASE( IO_no_security ) {
  flusspferd::object IO = flusspferd::create_object();
  flusspferd::global().set_property("IO", IO);
  flusspferd::io::load_io(IO);

  const char* js = "f = new IO.File('test/fixtures/file1'); f.readWhole()";
  flusspferd::root_value v;
  BOOST_CHECK_THROW( 
    v = flusspferd::evaluate(js, __FILE__, __LINE__),
    flusspferd::exception);
}

BOOST_AUTO_TEST_CASE( test_file_read ) {
  flusspferd::object IO = flusspferd::create_object();
  flusspferd::global().set_property("IO", IO);
  flusspferd::io::load_io(IO);

  flusspferd::security::create(flusspferd::global());

  const char* js = "f = new IO.File('test/fixtures/file1'); f.readWhole()";
  flusspferd::root_value v;
  BOOST_CHECK_NO_THROW( v = flusspferd::evaluate(js, __FILE__, __LINE__) );
  BOOST_CHECK_EQUAL(v.to_string().c_str(), "foobar\nbaz\n");

  js = "f = new IO.File('test/fixtures/file1'); f.readWhole()";
  BOOST_CHECK_NO_THROW( v = flusspferd::evaluate(js, __FILE__, __LINE__) );
  BOOST_CHECK_EQUAL(v.to_string().c_str(), "foobar\nbaz\n");

  flusspferd::gc();
}

#else

BOOST_AUTO_TEST_CASE( test_stub ) {
  BOOST_REQUIRE(true);
}
#endif

BOOST_AUTO_TEST_SUITE_END()

