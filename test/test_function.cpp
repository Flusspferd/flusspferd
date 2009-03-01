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

#include "flusspferd/function.hpp"
#include "flusspferd/native_function.hpp"
#include "flusspferd/native_function_base.hpp"

#include "flusspferd/create.hpp"
#include "flusspferd/string.hpp"
#include "flusspferd/string_io.hpp"

#include "test_environment.hpp"

namespace {
  flusspferd::string root_native_function_(flusspferd::object&, int) {
    return "Hallo Welt!";
  }

  struct function_struct : flusspferd::native_function_base {
    function_struct(unsigned arity, std::string const &name)
      : flusspferd::native_function_base(arity, name)
    { }
    function_struct() { }

    void call(flusspferd::call_context &x) {
      x.result = flusspferd::value(387);
    }
  };
}

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( root_native_function ) {
  flusspferd::root_function f_x(flusspferd::create_native_function(
    &root_native_function_, std::string("rnf")));
  
  BOOST_CHECK_EQUAL(f_x.arity(), 1ul);
  BOOST_CHECK_EQUAL(f_x.name(), "rnf");
}

BOOST_AUTO_TEST_SUITE_END()
