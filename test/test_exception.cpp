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

#include "flusspferd/create_on.hpp"
#include "flusspferd/value_io.hpp"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/assign/list_of.hpp>

#include "test_environment.hpp"

namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;
using namespace flusspferd;
using namespace flusspferd::param;
using boost::assign::list_of;

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

static void thrower(flusspferd::value v) {
  throw v;
}

BOOST_AUTO_TEST_CASE( throw_values ) {

  object g = flusspferd::global();
  flusspferd::create_on(g)
    .create<function>(
      _name = "cpp_thrower",
      _function = &thrower
    )
    .create<function>(
      _name = "js_thrower",
      _function = "throw arguments[0];"
    )
    .create<function>(
      _name = "js_catcher",
      _argument_names = list_of("meth")("val"),
      _function = "try { meth(val); return (void 0)} catch (e) { return e }"
    );


  value v = g.call("js_catcher", g.get_property("cpp_thrower"), 123);

  BOOST_CHECK_EQUAL(v.get_int(), 123);

  v = g.call("js_catcher", g.get_property("js_thrower"), 123);

  BOOST_CHECK_EQUAL(v.get_int(), 123);

  v = g.call("js_catcher", g.get_property("cpp_thrower"), g);

  BOOST_CHECK(v.is_object());
  BOOST_CHECK_EQUAL(v, value(g));

  v = g.call("js_catcher", g.get_property("js_thrower"), g);

  BOOST_CHECK(v.is_object());
  BOOST_CHECK_EQUAL(v, value(g));
}

BOOST_AUTO_TEST_SUITE_END()
