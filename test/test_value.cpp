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

#include "flusspferd/value.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/string_io.hpp"
#include "flusspferd/object.hpp"

#include <sstream>

#include "test_environment.hpp"

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( void_value ) {
  flusspferd::value void_value;
  BOOST_CHECK(void_value.is_undefined());
  BOOST_CHECK(!void_value.is_null());
  BOOST_CHECK(!void_value.is_boolean());
  BOOST_CHECK(!void_value.is_int());
  BOOST_CHECK(!void_value.is_double());
  BOOST_CHECK(!void_value.is_number());
  BOOST_CHECK(!void_value.is_string());
  BOOST_CHECK(!void_value.is_object());

  BOOST_CHECK_NE(void_value.to_number(), void_value.to_number());
}

BOOST_AUTO_TEST_CASE( null_value ) {
  flusspferd::object null_object;
  BOOST_REQUIRE(null_object.is_null());

  flusspferd::value null_value(null_object);
  BOOST_CHECK(!null_value.is_undefined());
  BOOST_CHECK(null_value.is_null());
  BOOST_CHECK(null_value.is_object());
  BOOST_CHECK(!null_value.is_boolean());
  BOOST_CHECK(!null_value.is_string());
  BOOST_CHECK(!null_value.is_number());
  BOOST_CHECK(!null_value.is_int());
  BOOST_CHECK(!null_value.is_double());

  BOOST_CHECK_EQUAL(null_value.to_number(), 0);
}

BOOST_AUTO_TEST_CASE( boolean_value ) {
  flusspferd::value boolean_value(false);
  BOOST_CHECK(boolean_value.is_boolean());
  BOOST_CHECK(!boolean_value.get_boolean());

  BOOST_CHECK(!boolean_value.is_undefined());
  BOOST_CHECK(!boolean_value.is_null());
  BOOST_CHECK(!boolean_value.is_number());
  BOOST_CHECK(!boolean_value.is_int());
  BOOST_CHECK(!boolean_value.is_double());
  BOOST_CHECK(!boolean_value.is_object());
  BOOST_CHECK(!boolean_value.is_string());

  BOOST_CHECK_EQUAL(boolean_value.to_number(), 0);

  boolean_value = true;
  BOOST_CHECK(boolean_value.is_boolean());
  BOOST_CHECK(boolean_value.get_boolean());

  BOOST_CHECK_EQUAL(boolean_value.to_number(), 1);
}

BOOST_AUTO_TEST_CASE( value_to_boolean ) {
  flusspferd::value v(12);
  BOOST_CHECK(v.is_number());
  BOOST_CHECK(v.to_boolean());
  v = 0;
  BOOST_CHECK(v.is_number());
  BOOST_CHECK(!v.to_boolean());
}

BOOST_AUTO_TEST_CASE( int_value ) {
  int const X = 4;

  flusspferd::value int_value(X);
  BOOST_CHECK(int_value.is_number());
  BOOST_CHECK(int_value.is_int());
  BOOST_CHECK(!int_value.is_double());
  BOOST_CHECK(!int_value.is_undefined());
  BOOST_CHECK(!int_value.is_null());
  BOOST_CHECK(!int_value.is_boolean());
  BOOST_CHECK(!int_value.is_object());
  BOOST_CHECK(!int_value.is_string());

  BOOST_CHECK_EQUAL(int_value.get_int(), X);
  BOOST_CHECK_EQUAL(int_value.to_number(), double(X));
}

BOOST_AUTO_TEST_CASE( double_value ) {
  double const X = 4.5;

  flusspferd::value double_value(X);
  BOOST_CHECK(double_value.is_number());
  BOOST_CHECK(double_value.is_double());
  BOOST_CHECK(!double_value.is_int());
  BOOST_CHECK(!double_value.is_undefined());
  BOOST_CHECK(!double_value.is_null());
  BOOST_CHECK(!double_value.is_boolean());
  BOOST_CHECK(!double_value.is_object());
  BOOST_CHECK(!double_value.is_string());

  BOOST_CHECK_EQUAL(double_value.get_double(), X);
  BOOST_CHECK_EQUAL(double_value.to_number(), X);
}

BOOST_AUTO_TEST_CASE( string_value ) {
  flusspferd::string s = "Hallo";
  flusspferd::value v(s);
  BOOST_CHECK(v.is_string());
  BOOST_CHECK(!v.is_object());
  BOOST_CHECK(!v.is_number());
  BOOST_CHECK(!v.is_double());
  BOOST_CHECK(!v.is_int());
  BOOST_CHECK(!v.is_undefined());
  BOOST_CHECK(!v.is_null());
  BOOST_CHECK(!v.is_boolean());

  BOOST_CHECK_EQUAL(v.get_string(), s);
  BOOST_CHECK_EQUAL(v.to_string(), s);

  std::stringstream ss1;
  ss1 << v;
  std::stringstream ss2;
  ss2 << s;
  BOOST_CHECK_EQUAL(ss1.str(), ss2.str());
}

BOOST_AUTO_TEST_SUITE_END()

