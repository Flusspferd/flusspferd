// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:enc=utf-8:
/*
The MIT License

Copyright (c) 2009 Aristid Breitkreuz, Ash Berlin, Rüdiger Sonderfeld

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

#include "flusspferd/array.hpp"

#include "flusspferd/create.hpp"
#include "flusspferd/value.hpp"
#include "test_environment.hpp"

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( array ) {
  std::size_t const array_size = 10;
  flusspferd::array a = flusspferd::create_array(array_size);
  BOOST_CHECK_EQUAL(a.length(), array_size);
  BOOST_CHECK_EQUAL(a.size(), a.length());
  int const value0 = 10;
  a.set_element(0, flusspferd::value(value0));
  BOOST_CHECK(a.get_element(0).is_int());
  BOOST_CHECK_EQUAL(a.get_element(0).get_int(), value0);
}

BOOST_AUTO_TEST_SUITE_END()
