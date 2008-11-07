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

#include "flusspferd/value.hpp"
#include "flusspferd/object.hpp"
#include <boost/test/unit_test.hpp>

boost::unit_test::test_suite *init_unit_test_suite(int, char **) {
  return 0;
}

BOOST_AUTO_TEST_CASE( void_value ) {
  flusspferd::value void_value;
  BOOST_CHECK(void_value.is_void());
  BOOST_CHECK(!void_value.is_null());
}

BOOST_AUTO_TEST_CASE( null_value ) {
  flusspferd::object null_object;
  BOOST_REQUIRE(!null_object.is_valid());

  flusspferd::value null_value(null_object);
  BOOST_CHECK(!null_value.is_void());
  BOOST_CHECK(null_value.is_null());
}