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

#include "flusspferd/current_context_scope.hpp"
#include "test_environment.hpp"

BOOST_TEST_DONT_PRINT_LOG_VALUE(flusspferd::context)

BOOST_AUTO_TEST_CASE( current_context_scope ) {
  flusspferd::init &init = flusspferd::init::initialize();

  flusspferd::context old_context(init.current_context());

  flusspferd::context context(flusspferd::context::create());

  BOOST_CHECK_NE(context, init.current_context());

  {
    flusspferd::current_context_scope scope(context);

    BOOST_CHECK_EQUAL(context, init.current_context());

    {
      flusspferd::context context2(flusspferd::context::create());

      flusspferd::current_context_scope scope(context2);

      BOOST_CHECK_EQUAL(context2, init.current_context());
    }

    BOOST_CHECK_EQUAL(context, init.current_context());
  }

  BOOST_CHECK_EQUAL(old_context, init.current_context());
}

BOOST_FIXTURE_TEST_CASE( context_in_fixture, context_fixture ) {
  BOOST_CHECK(flusspferd::current_context().is_valid());
}
