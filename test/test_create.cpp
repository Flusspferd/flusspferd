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
#include "flusspferd/create/object.hpp"
#include "flusspferd/create/array.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/create/native_object.hpp"
#include "flusspferd/create/native_function.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/native_function_base.hpp"
#include "test_environment.hpp"
#include <boost/assign/list_of.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>

using namespace flusspferd::param;
using namespace boost::assign;

FLUSSPFERD_CLASS_DESCRIPTION(
  my_class,
  (constructor_name, "MyClass")
  (full_name, "test_create.MyClass")
)
{
public:
  enum constructor_choice_t {
    obj_only,
    ab
  };

  constructor_choice_t constructor_choice;
  int a;
  std::string b;

  my_class(object const &obj)
    : base_type(obj),
      constructor_choice(obj_only)
  {}

  my_class(object const &obj, int a, std::string const &b)
    : base_type(obj),
      constructor_choice(ab),
      a(a),
      b(b)
  {}
};

struct my_functor : flusspferd::native_function_base {
  bool x;

  my_functor() : x(true) {}
  my_functor(int) : x(false) {}

  void call(flusspferd::call_context &) {
    set_property("called", x);
  }
};

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

BOOST_AUTO_TEST_CASE( function ) {
  flusspferd::local_root_scope scope;

  flusspferd::function f = flusspferd::create<flusspferd::function>();
  flusspferd::value v;
  BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global()));
  BOOST_CHECK_EQUAL(v, flusspferd::value());

  f = flusspferd::create<flusspferd::function>(
    _argument_names = list_of("a"),
    _source = "return a * 2");
  BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global(), 4));
  BOOST_CHECK_EQUAL(v, flusspferd::value(8));

  f = flusspferd::create<flusspferd::function>(
    "name",
    "return x + y",
    list_of("x")("y"),
    "file",
    666);    
  BOOST_CHECK_EQUAL(f.name(), "name");
  BOOST_CHECK_EQUAL(f.arity(), 2);
  BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global(), 1, 2));
  BOOST_CHECK_EQUAL(v, flusspferd::value(3));
}

BOOST_AUTO_TEST_CASE( MyClass ) {
  flusspferd::local_root_scope scope;

  my_class &obj = flusspferd::create<my_class>();
  BOOST_CHECK(!obj.is_null());
  BOOST_CHECK_EQUAL(
    &obj,
    &flusspferd::get_native<my_class>(flusspferd::object(obj)));
  BOOST_CHECK_EQUAL(obj.constructor_choice, my_class::obj_only);

  my_class &obj2 = flusspferd::create<my_class>(
    boost::fusion::make_vector(5, "hey"));
  BOOST_CHECK(!obj2.is_null());
  BOOST_CHECK_EQUAL(obj2.constructor_choice, my_class::ab);
  BOOST_CHECK_EQUAL(obj2.a, 5);
  BOOST_CHECK_EQUAL(obj2.b, "hey");
}

BOOST_AUTO_TEST_CASE( MyFunctor ) {
  flusspferd::local_root_scope scope;

  flusspferd::object f;
  BOOST_CHECK_NO_THROW(f = flusspferd::create<my_functor>());
  BOOST_CHECK(!f.is_null());
  BOOST_CHECK_EQUAL(f.get_property("called"), flusspferd::value());
  BOOST_CHECK_NO_THROW(f.call(flusspferd::global()));
  BOOST_CHECK_EQUAL(f.get_property("called"), flusspferd::value(true));

  BOOST_CHECK_NO_THROW(
    f = flusspferd::create<my_functor>(boost::fusion::make_vector(0)));
  BOOST_CHECK(!f.is_null());
  BOOST_CHECK_EQUAL(f.get_property("called"), flusspferd::value());
  BOOST_CHECK_NO_THROW(f.call(flusspferd::global()));
  BOOST_CHECK_EQUAL(f.get_property("called"), flusspferd::value(false));
}

BOOST_AUTO_TEST_CASE( container ) {
  flusspferd::local_root_scope scope;
  boost::optional<flusspferd::property_attributes> attr;

  flusspferd::object cont = flusspferd::create<flusspferd::object>();
  BOOST_CHECK(!cont.is_null());

  flusspferd::object o =
    flusspferd::create<flusspferd::object>(
      _container = cont,
      _name = "o"
    );
  BOOST_CHECK(!o.is_null());
  BOOST_CHECK_EQUAL(cont.get_property_object("o"), o);
  BOOST_CHECK(attr = cont.get_property_attributes("o"));
  BOOST_CHECK_EQUAL(attr->flags, flusspferd::dont_enumerate);

  flusspferd::array a =
    flusspferd::create<flusspferd::array>(
      _name = "a",
      5,
      _container = cont,
      _attributes = flusspferd::no_property_flag);
  BOOST_CHECK_EQUAL(a.length(), 5);
  BOOST_CHECK_EQUAL(cont.get_property_object("a"), a);
  BOOST_CHECK(attr = cont.get_property_attributes("a"));
  BOOST_CHECK_EQUAL(attr->flags, flusspferd::no_property_flag);

  flusspferd::function f =
    flusspferd::create<flusspferd::function>(
      "f",
      _container = cont,
      _source = "return x+1",
      _argument_names = list_of("x"));
  BOOST_CHECK_EQUAL(f.call(flusspferd::global(), 1), flusspferd::value(2));
  BOOST_CHECK_EQUAL(cont.get_property("f"), f);
  BOOST_CHECK_EQUAL(cont.call("f", 2), flusspferd::value(3));
  BOOST_CHECK(attr = cont.get_property_attributes("f"));
  BOOST_CHECK_EQUAL(attr->flags, flusspferd::dont_enumerate);

  my_class &m = flusspferd::create<my_class>(_name = "m", _container = cont);
  BOOST_CHECK(!m.is_null());
  BOOST_CHECK_EQUAL(cont.get_property("m"), m);
  BOOST_CHECK(attr = cont.get_property_attributes("m"));
  BOOST_CHECK_EQUAL(attr->flags, flusspferd::dont_enumerate);
}

BOOST_AUTO_TEST_SUITE_END()
