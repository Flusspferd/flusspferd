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

#include "flusspferd/property_iterator.hpp"
#include "flusspferd/object.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/value_io.hpp"
#include "test_environment.hpp"

BOOST_TEST_DONT_PRINT_LOG_VALUE(flusspferd::object) //FIXME?
BOOST_TEST_DONT_PRINT_LOG_VALUE(flusspferd::property_iterator) //FIXME?

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( property_iterator ) {
  flusspferd::object obj = flusspferd::create<flusspferd::object>();
  flusspferd::value const name(flusspferd::string("foobar"));
  flusspferd::value const v(409);
  obj.set_property(name, v);

  flusspferd::property_iterator i = obj.begin();
  BOOST_CHECK_EQUAL(i, obj.begin());
  flusspferd::property_iterator j = obj.end();
  BOOST_CHECK_EQUAL(j, obj.end());
  flusspferd::property_iterator k;
  k = j;
  BOOST_CHECK_EQUAL(k, j);
  flusspferd::property_iterator l = i;
  BOOST_CHECK_EQUAL(l, i);
  k.swap(l);
  BOOST_CHECK_EQUAL(k, i);
  BOOST_CHECK_EQUAL(l, j);

  BOOST_CHECK_NE(*i, v);
  BOOST_CHECK_EQUAL(*i, name);

  BOOST_CHECK_EQUAL(++i, j);
}

BOOST_AUTO_TEST_SUITE_END()
