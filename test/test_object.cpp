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

#include "flusspferd/object.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/property_iterator.hpp"
#include "flusspferd/local_root_scope.hpp"
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
  flusspferd::object const &plain_object =
    flusspferd::create<flusspferd::object>();
  BOOST_CHECK(!plain_object.is_null());

  BOOST_CHECK_EQUAL(plain_object, plain_object);
  BOOST_CHECK_NE(plain_object, flusspferd::object());

  flusspferd::object object_copy(plain_object);
  BOOST_CHECK(!object_copy.is_null());

  BOOST_CHECK_EQUAL(plain_object, object_copy);

  flusspferd::object const &plain_object2 =
    flusspferd::create<flusspferd::object>();
  BOOST_CHECK_NE(plain_object, plain_object2);
}

BOOST_AUTO_TEST_CASE( object_property ) {
  flusspferd::object obj = flusspferd::create<flusspferd::object>();
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
  flusspferd::object obj = flusspferd::create<flusspferd::object>();
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

  flusspferd::object &obj = invalid_object;

#define X(a) BOOST_CHECK_THROW((a), flusspferd::exception)

  X(obj.seal(true));
  X(obj.parent());
  X(obj.prototype());
  X(obj.set_parent(flusspferd::object()));
  X(obj.set_prototype(flusspferd::object()));  
  X((obj.apply(flusspferd::global(), flusspferd::arguments())))
  X((obj.call("toString", flusspferd::arguments())));
  X((obj.call(flusspferd::global(), flusspferd::arguments())));
  X(obj.call(flusspferd::arguments()));
  X(obj.define_property("abc"));
  X(obj.define_property(flusspferd::string()));
  X(obj.define_property(std::string("abc")));
  X(obj.set_property("abc", flusspferd::value()));
  X(obj.set_property(flusspferd::value(3), flusspferd::value()));
  X(obj.set_property(std::string("abc"), flusspferd::value()));
  X(obj.get_property("abc"));
  X(obj.get_property(std::string("abc")));
  X(obj.get_property(flusspferd::value(3)));
  X(obj.has_property("abc"));
  X(obj.has_property(std::string("abc")));
  X(obj.has_property(flusspferd::value(3)));
  X(obj.has_own_property("abc"));
  X(obj.has_own_property(std::string("abc")));
  X(obj.has_own_property(flusspferd::value(3)));
  X(obj.delete_property("abc"));
  X(obj.delete_property(std::string("abc")));
  X(obj.delete_property(flusspferd::value(3)));
  X(obj.begin());
  X(obj.end());
  BOOST_CHECK_EQUAL(obj.is_array(), false);

  X(obj.get_property_attributes("abc"));
  X(obj.get_property_attributes(std::string("abc")));
  X(obj.get_property_attributes(flusspferd::string()));

  //TODO
#undef X
}

BOOST_AUTO_TEST_CASE( recursive_loop_on_set_property ) {
  flusspferd::root_object object(flusspferd::create<flusspferd::object>());

  object.set_property( flusspferd::string(), flusspferd::string() );
}

BOOST_AUTO_TEST_CASE( set_properties ) {
  flusspferd::root_object object(flusspferd::create<flusspferd::object>());

  object.set_properties(flusspferd::value(1),2)("3",4);

  BOOST_CHECK_EQUAL(object.get_property("1"), flusspferd::value(2));
  BOOST_CHECK_EQUAL(object.get_property(flusspferd::value(3)), flusspferd::value(4));
}

BOOST_AUTO_TEST_CASE( define_properties ) {
  flusspferd::root_object object(flusspferd::create<flusspferd::object>());

  object.define_properties(flusspferd::read_only_property)(flusspferd::value(1),2)("3",4)("5");
  object.set_property("1", 5); //no effect

  BOOST_CHECK_EQUAL(object.get_property("1"), flusspferd::value(2));
  BOOST_CHECK_EQUAL(object.get_property(flusspferd::value(3)), flusspferd::value(4));
  BOOST_CHECK_EQUAL(object.get_property("5"), flusspferd::value());
}

BOOST_AUTO_TEST_SUITE_END()
