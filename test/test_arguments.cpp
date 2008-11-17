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

#include "flusspferd/arguments.hpp"
#include "flusspferd/value.hpp"
#include "flusspferd/value_io.hpp"
#include "test_environment.hpp"


BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( arguments_empty ) {
  flusspferd::arguments a;
  BOOST_CHECK(a.empty());
  BOOST_CHECK_EQUAL(a.size(), 0ul);
}

BOOST_AUTO_TEST_CASE( arguments_push_back_front ) {
  flusspferd::arguments a;
  BOOST_CHECK(a.front().is_void());
  BOOST_CHECK(a.back().is_void());
  flusspferd::value v0(299);
  a.push_back(v0);
  BOOST_CHECK_EQUAL(a.front(), v0);
  BOOST_CHECK_EQUAL(a.back(), v0);
  flusspferd::value v1(3.5);
  a.push_back(v1);
  BOOST_CHECK_EQUAL(a.front(), v0);
  BOOST_CHECK_EQUAL(a.back(), v1);
}

BOOST_AUTO_TEST_CASE( arguments_init_value_vector ) {
  std::vector<flusspferd::value> v;
  typedef std::vector<flusspferd::value>::const_iterator viter;
  v.push_back(flusspferd::value(true));
  v.push_back(flusspferd::value(10));
  v.push_back(flusspferd::value(3.0));
  v.push_back(flusspferd::value(false));

  flusspferd::arguments a(v);
  typedef flusspferd::arguments::iterator aiter;

  BOOST_CHECK_EQUAL(a.size(), v.size());

  aiter i = a.begin();
  BOOST_CHECK(i == a.begin());
  aiter const aend = a.end();
  viter j = v.begin();
  viter const vend = v.end();

  for(; i != aend && j != vend; ++j, ++i) {
    BOOST_CHECK_EQUAL(*i, *j);
  }
}

BOOST_AUTO_TEST_CASE( arguments_op_access ) {
  std::vector<flusspferd::value> v;
  v.push_back(flusspferd::value(true));
  v.push_back(flusspferd::value(10));
  v.push_back(flusspferd::value(3.0));
  v.push_back(flusspferd::value(false));

  flusspferd::arguments a(v);
  std::size_t const asize = a.size();
  BOOST_CHECK_EQUAL(asize, v.size());

  for(std::size_t i = 0; i < asize; ++i) {
    BOOST_CHECK_EQUAL(a[i], v[i]);
  }
}

BOOST_AUTO_TEST_CASE( arguments_op_access_offbyone ) {
  flusspferd::arguments a;
  BOOST_CHECK(a[1].is_void());
}

BOOST_AUTO_TEST_CASE( arguments_op_copy ) {
  std::vector<flusspferd::value> v;
  v.push_back(flusspferd::value(true));
  v.push_back(flusspferd::value(10));
  v.push_back(flusspferd::value(3.0));
  v.push_back(flusspferd::value(false));

  flusspferd::arguments a(v);
  std::size_t const asize = a.size();
  BOOST_CHECK_EQUAL(asize, v.size());

  flusspferd::arguments b;
  b = a;
  BOOST_CHECK_EQUAL(asize, b.size());

  for(std::size_t i = 0; i < asize; ++i) {
    BOOST_CHECK_EQUAL(a[i], b[i]);
  }
}

BOOST_AUTO_TEST_CASE( arguments_copy_ctor ) {
  std::vector<flusspferd::value> v;
  v.push_back(flusspferd::value(true));
  v.push_back(flusspferd::value(10));
  v.push_back(flusspferd::value(3.0));
  v.push_back(flusspferd::value(false));

  flusspferd::arguments a(v);
  std::size_t const asize = a.size();
  BOOST_CHECK_EQUAL(asize, v.size());

  flusspferd::arguments b = a;
  BOOST_CHECK_EQUAL(asize, b.size());

  for(std::size_t i = 0; i < asize; ++i) {
    BOOST_CHECK_EQUAL(a[i], b[i]);
  }
}

BOOST_AUTO_TEST_SUITE_END()
