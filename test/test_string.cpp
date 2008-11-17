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

#include "flusspferd/string.hpp"
#include "flusspferd/string_io.hpp"
#include "flusspferd/exception.hpp"

#include <iostream>
#include <cstring>
#include <string>

#include "test_environment.hpp"

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( empty_string ) {
  flusspferd::string s, t;
  BOOST_CHECK(s.length() == 0);
  BOOST_CHECK(s.to_string().empty());
  BOOST_CHECK(std::strlen(s.c_str()) == 0);
  BOOST_CHECK_EQUAL(s, t);
}

BOOST_AUTO_TEST_CASE( cstring ) {
  char const * const test = "Hallo Welt!\n";
  flusspferd::string s(test);
  BOOST_CHECK_EQUAL(s.length(), std::strlen(test));
  BOOST_CHECK(std::strcmp(s.c_str(), test) == 0);
}

BOOST_AUTO_TEST_CASE( std_string ) {
  std::string test = "Hallo Welt!\n";
  flusspferd::string s(test);
  BOOST_CHECK_EQUAL(s.length(), test.length());
  BOOST_CHECK_EQUAL(s.to_string(), test);
  BOOST_CHECK(std::strcmp(s.c_str(), test.c_str()) == 0);
}

BOOST_AUTO_TEST_CASE( copy_op ) {
  flusspferd::string const a("Hallo Welt!\n");
  flusspferd::string b;
  b = a;
  BOOST_CHECK_EQUAL(a, b);
}

BOOST_AUTO_TEST_CASE( op_less ) {
  std::string const ss1 = "aaa";
  std::string const ss2 = "aab";
  flusspferd::string fs1 = ss1;
  flusspferd::string fs2 = ss2;

  BOOST_CHECK( (fs1 < fs2) == (ss1 < ss2) );
  BOOST_CHECK( (fs2 < fs1) == (ss2 < ss1) );
}

BOOST_AUTO_TEST_CASE( string_substr ) {
  std::string const str = "Ba bo bu bi bo bz";
  flusspferd::string s = str;
  BOOST_CHECK_EQUAL(s.substr(3, 5), str.substr(3, 5));
}

BOOST_AUTO_TEST_CASE( string_io ) {
  std::stringstream ss;
  flusspferd::string s("Hallo Welt!");
  ss << s;
  BOOST_CHECK_EQUAL(s.to_string(), ss.str());
}

BOOST_AUTO_TEST_SUITE_END()
