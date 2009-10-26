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

#include "flusspferd/create.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/init.hpp"
#include "test_environment.hpp"
#include <boost/assign/list_of.hpp>

using namespace flusspferd::param;
using namespace boost::assign;

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( object ) {
  flusspferd::local_root_scope scope;

  flusspferd::object o1(flusspferd::create<flusspferd::object>());
  BOOST_CHECK(!o1.is_null());
  BOOST_CHECK_EQUAL(
    o1.prototype(), flusspferd::current_context().prototype(""));

  flusspferd::object o2(
    flusspferd::create<flusspferd::object>(_prototype = o1));
  BOOST_CHECK(!o2.is_null());
  BOOST_CHECK_EQUAL(o2.prototype(), o1);

  o2 = flusspferd::create<flusspferd::object>(o1);
  BOOST_CHECK(!o2.is_null());
  BOOST_CHECK_EQUAL(o2.prototype(), o1);

  flusspferd::object o3(
    flusspferd::create<flusspferd::object>(_parent = o2));
  BOOST_CHECK(!o3.is_null());
  BOOST_CHECK_EQUAL(o3.parent(), o2);

  flusspferd::object o4(
    flusspferd::create<flusspferd::object>(o1, o2));
  BOOST_CHECK(!o4.is_null());
  BOOST_CHECK_EQUAL(o4.prototype(), o1);
  BOOST_CHECK_EQUAL(o4.parent(), o2);

  o4 = flusspferd::create<flusspferd::object>(_parent = o2, _prototype = o1);
  BOOST_CHECK(!o4.is_null());
  BOOST_CHECK_EQUAL(o4.prototype(), o1);
  BOOST_CHECK_EQUAL(o4.parent(), o2);
}

BOOST_AUTO_TEST_CASE( array ) {
  flusspferd::local_root_scope scope;

  flusspferd::array a(flusspferd::create<flusspferd::array>());
  BOOST_CHECK_EQUAL(a.length(), 0);

  a = flusspferd::create<flusspferd::array>(_length = 5);
  BOOST_CHECK_EQUAL(a.length(), 5);

  a = flusspferd::create<flusspferd::array>(3);
  BOOST_CHECK_EQUAL(a.length(), 3);

  a = flusspferd::create<flusspferd::array>(_contents = list_of(1)(2));
  BOOST_CHECK_EQUAL(a.length(), 2);
  BOOST_CHECK_EQUAL(a.get_element(0), flusspferd::value(1));
  BOOST_CHECK_EQUAL(a.get_element(1), flusspferd::value(2));

  a = flusspferd::create<flusspferd::array>(list_of(9));
  BOOST_CHECK_EQUAL(a.length(), 1);
  BOOST_CHECK_EQUAL(a.get_element(0), flusspferd::value(9));
}

BOOST_AUTO_TEST_SUITE_END()
