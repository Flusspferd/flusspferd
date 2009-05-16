// vim:ts=2:sw=2:expandtab:autoindent:filetype=cpp:
/*
Copyright (c) 2008, 2009 Aristid Breitkreuz, Ash Berlin, Ruediger Sonderfeld

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

#include "flusspferd/init.hpp"
#include <boost/test/unit_test.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE(flusspferd::context)

BOOST_AUTO_TEST_CASE( initialize ) {
  flusspferd::init *init = 0;
  init = &flusspferd::init::initialize();
  BOOST_CHECK_NE(init, (flusspferd::init*) 0);

  flusspferd::context &context = init->current_context();
  BOOST_CHECK(!context.is_valid());
}

BOOST_AUTO_TEST_CASE( enter_leave ) {
  flusspferd::init &init = flusspferd::init::initialize();

  flusspferd::context old_context(init.current_context());

  flusspferd::context context(flusspferd::context::create());

  BOOST_CHECK_NE(context, init.current_context());

  BOOST_CHECK_EQUAL(init.enter_current_context(context), old_context);

  BOOST_CHECK_EQUAL(context, init.current_context());

  BOOST_CHECK(!init.leave_current_context(old_context));

  BOOST_CHECK(init.leave_current_context(context));

  BOOST_CHECK_EQUAL(old_context, init.current_context());
}

