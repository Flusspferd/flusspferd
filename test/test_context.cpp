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

#include "flusspferd/context.hpp"
#include "flusspferd/spidermonkey/context.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( null_context ) {
  flusspferd::context null_context;
  BOOST_CHECK(!null_context.is_valid());
}

BOOST_AUTO_TEST_CASE( copy_null_context ) {
  flusspferd::context original_null_context;
  BOOST_REQUIRE(!original_null_context.is_valid());
  flusspferd::context null_context;
  BOOST_CHECK(!null_context.is_valid());
}

BOOST_AUTO_TEST_CASE( create_context ) {
  flusspferd::context const &context = flusspferd::context::create();
  BOOST_CHECK(context.is_valid());
}

BOOST_AUTO_TEST_CASE( copy_create_context ) {
  flusspferd::context const &original_context = flusspferd::context::create();
  BOOST_REQUIRE(original_context.is_valid());
  flusspferd::context context(original_context);
  BOOST_CHECK(context.is_valid());
}

BOOST_AUTO_TEST_SUITE( spidermonkey )

BOOST_AUTO_TEST_CASE( direct_null_context ) {
  flusspferd::context const &null_context = flusspferd::Impl::wrap_context(0);
  BOOST_CHECK(!null_context.is_valid());
}

BOOST_AUTO_TEST_SUITE_END()

