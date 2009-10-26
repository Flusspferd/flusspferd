// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:enc=utf-8:
/*
The MIT License

Copyright (c) 2009 Flusspferd contributors (see "CONTRIBUTORS" or
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

#include "flusspferd/array.hpp"

#include "flusspferd/create.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/value_io.hpp"
#include "test_environment.hpp"
#include <boost/assign/list_of.hpp>

#include <iostream>//FIXME

BOOST_TEST_DONT_PRINT_LOG_VALUE(flusspferd::array::iterator)

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( array ) {
  std::size_t const array_size = 10;
  flusspferd::array a = flusspferd::create<flusspferd::array>(array_size);
  BOOST_CHECK_EQUAL(a.length(), array_size);
  BOOST_CHECK_EQUAL(a.size(), a.length());

  int const value0 = 10;
  a.set_element(0, flusspferd::value(value0));
  BOOST_CHECK(a.get_element(0).is_int());
  BOOST_CHECK_EQUAL(a.get_element(0).get_int(), value0);

  for(std::size_t i = 0; i < array_size; ++i) {
    a.set_element(i, static_cast<int>(i));
  }
  for(std::size_t i = 0; i < array_size; ++i) {
    BOOST_CHECK_EQUAL(a.get_element(i),flusspferd::value(static_cast<int>(i)));
  }

  {
    std::size_t const new_array_size = 15;
    a.set_length(new_array_size);
    BOOST_CHECK_EQUAL(a.length(), new_array_size);
  }

  {
    std::size_t const new_array_size = 5;
    a.set_length(new_array_size);
    BOOST_CHECK_EQUAL(a.length(), new_array_size);
  }
}

BOOST_AUTO_TEST_CASE( push) {
  flusspferd::array a = flusspferd::create<flusspferd::array>();
  a.push(0, 1, 2);
  BOOST_CHECK_EQUAL(a.length(), 3);
  for (int i = 0; i < 3; ++i)
    BOOST_CHECK_EQUAL(a.get_element(i), flusspferd::value(i));
}

BOOST_AUTO_TEST_CASE( create_range ) {
  flusspferd::array a = flusspferd::create<flusspferd::array>(
    boost::assign::list_of
    (1)
    (2)
    (3));
  BOOST_CHECK_EQUAL(a.length(), 3);
  for (int i = 0; i < 3; ++i)
    BOOST_CHECK_EQUAL(a.get_element(i), flusspferd::value(i + 1));
}

BOOST_AUTO_TEST_CASE( array_iterator ) {
  std::size_t const array_size = 10;
  flusspferd::array a = flusspferd::create<flusspferd::array>();

  BOOST_CHECK_EQUAL(a.length(), 0);

  for(std::size_t i = 0; i < array_size; ++i) {
    a.set_element(i, flusspferd::value(static_cast<int>(i)));
  }

  BOOST_CHECK_EQUAL(a.length(), array_size);

  flusspferd::array::iterator i = a.begin();
  flusspferd::array::iterator const end = a.end();
  BOOST_REQUIRE_NE(i, end);
  BOOST_REQUIRE_EQUAL(end - i, static_cast<ptrdiff_t>(array_size));
  int j = 0;
  for(; i != end; ++i, ++j) {
    BOOST_REQUIRE(i->is_int());
    BOOST_CHECK_EQUAL(i->get_int(), j);
  }
}

BOOST_AUTO_TEST_SUITE_END()
