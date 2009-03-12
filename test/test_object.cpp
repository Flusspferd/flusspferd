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

#include "flusspferd/object.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/value_io.hpp"
#include "test_environment.hpp"

BOOST_TEST_DONT_PRINT_LOG_VALUE(flusspferd::object) //FIXME?

BOOST_FIXTURE_TEST_SUITE( with_context, context_fixture )

BOOST_AUTO_TEST_CASE( null_object ) {
  flusspferd::object null_object;
  BOOST_CHECK(null_object.is_null());

  BOOST_CHECK_EQUAL(null_object, null_object);

  flusspferd::object object_copy(null_object);
  BOOST_CHECK(null_object.is_null());

  BOOST_CHECK_EQUAL(null_object, object_copy);
}

BOOST_AUTO_TEST_CASE( plain_object ) {
  flusspferd::object const &plain_object = flusspferd::create_object();
  BOOST_CHECK(!plain_object.is_null());

  BOOST_CHECK_EQUAL(plain_object, plain_object);
  BOOST_CHECK_NE(plain_object, flusspferd::object());

  flusspferd::object object_copy(plain_object);
  BOOST_CHECK(!object_copy.is_null());

  BOOST_CHECK_EQUAL(plain_object, object_copy);

  flusspferd::object const &plain_object2 = flusspferd::create_object();
  BOOST_CHECK_NE(plain_object, plain_object2);
}

BOOST_AUTO_TEST_CASE( object_property ) {
  flusspferd::object obj = flusspferd::create_object();
  std::string const name = "foobar";
  flusspferd::value const v(409);
  obj.set_property(name, v);
  BOOST_CHECK(obj.has_property(name));
  BOOST_CHECK_EQUAL(obj.get_property(name), v);
  obj.delete_property(name);
  BOOST_CHECK(!obj.has_property(name));
  flusspferd::value const empty;
  BOOST_CHECK_EQUAL(obj.get_property(name), empty);
}

BOOST_AUTO_TEST_CASE( object_property_value ) {
  flusspferd::object obj = flusspferd::create_object();
  flusspferd::value const name = flusspferd::string("foobar");
  flusspferd::value const v(409);
  obj.set_property(name, v);
  BOOST_CHECK(obj.has_property(name));
  BOOST_CHECK_EQUAL(obj.get_property(name), v);
  obj.delete_property(name);
  BOOST_CHECK(!obj.has_property(name));
  flusspferd::value const empty;
  BOOST_CHECK_EQUAL(obj.get_property(name), empty);
}

BOOST_AUTO_TEST_CASE( function_as_object ) {
  flusspferd::value v = flusspferd::evaluate("(function () {})");
  BOOST_CHECK(!v.is_undefined());
  BOOST_CHECK(v.is_object());
  BOOST_CHECK(!v.is_null());
}

BOOST_AUTO_TEST_CASE( call_on_invalid ) {
  flusspferd::object invalid_object;
  BOOST_REQUIRE(invalid_object.is_null());
  BOOST_CHECK_THROW(invalid_object.parent(), flusspferd::exception);
  BOOST_CHECK_THROW(invalid_object.prototype(), flusspferd::exception);
  BOOST_CHECK_THROW(
    invalid_object.set_parent(flusspferd::object()), flusspferd::exception);
  BOOST_CHECK_THROW(
    invalid_object.set_prototype(flusspferd::object()), flusspferd::exception);
  BOOST_CHECK_THROW(
    (invalid_object.apply(flusspferd::global(), flusspferd::arguments())),
    flusspferd::exception);
  //TODO
}

BOOST_AUTO_TEST_SUITE_END()
