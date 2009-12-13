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
#include <boost/fusion/include/make_vector.hpp>

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

  void my_method_1(flusspferd::call_context &) {}
  double my_method_2(double x) {
    return x / 2;
  }
};

struct my_functor : flusspferd::native_function_base {
  typedef boost::mpl::true_ ignore_name_arity;

  bool x;

  my_functor(flusspferd::function const &obj)
    : flusspferd::native_function_base(obj), x(true) {}
  my_functor(flusspferd::function const &obj, int)
    : flusspferd::native_function_base(obj), x(false) {}

  void call(flusspferd::call_context &) {
    set_property("called", x);
  }
};

static int my_func_1(flusspferd::call_context &) {
  return 0;
}

static void my_func_2(flusspferd::call_context &)
{}

static int my_func_3(int x) {
  return x;
}

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
  flusspferd::root_function f;
  f = flusspferd::create<flusspferd::function>();
  flusspferd::root_value v;
  BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global()));
  BOOST_CHECK_EQUAL(v, flusspferd::value());

  f = flusspferd::create<flusspferd::function>(
    _argument_names = list_of("a"),
    _function = "return a * 2");
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

  f = flusspferd::create<flusspferd::function>(
    "name2",
    my_func_1,
    flusspferd::param::_signature =
      flusspferd::param::type< void (flusspferd::call_context&) >());
  BOOST_CHECK_EQUAL(f.name(), "name2");
  BOOST_CHECK_EQUAL(f.arity(), 0);
  BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global()));
  BOOST_CHECK_EQUAL(v, flusspferd::value());

  f = flusspferd::create<flusspferd::function>(
    "name3",
    my_func_2);
  BOOST_CHECK_EQUAL(f.name(), "name3");
  BOOST_CHECK_EQUAL(f.arity(), 0);
  BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global()));
  BOOST_CHECK_EQUAL(v, flusspferd::value());

  f = flusspferd::create<flusspferd::function>(
    "name4",
    my_func_3);
  BOOST_CHECK_EQUAL(f.name(), "name4");
  BOOST_CHECK_EQUAL(f.arity(), 1);
  BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global(), 12345));
  BOOST_CHECK_EQUAL(v, flusspferd::value(12345));

  f = flusspferd::create<flusspferd::method>(
    "name5",
    &my_class::my_method_1);
  BOOST_CHECK_EQUAL(f.name(), "name5");
  BOOST_CHECK_EQUAL(f.arity(), 0);
  BOOST_CHECK_NO_THROW(v = f.call(
    flusspferd::create<my_class>()));
  BOOST_CHECK_EQUAL(v, flusspferd::value());

  f = flusspferd::create<flusspferd::method>(
    "name6",
    &my_class::my_method_2);
  BOOST_CHECK_EQUAL(f.name(), "name6");
  BOOST_CHECK_EQUAL(f.arity(), 1);
  BOOST_CHECK_NO_THROW(v = f.call(
    flusspferd::create<my_class>(),
    5));
  BOOST_CHECK_EQUAL(v, flusspferd::value(2.5));
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
  flusspferd::root_object f;
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
      _function = "return x+1",
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

BOOST_AUTO_TEST_SUITE(create_on)

BOOST_AUTO_TEST_CASE(object) {
  flusspferd::create_on(flusspferd::global())
    .create<flusspferd::object>(flusspferd::param::_name = "my_obj")
    .create<flusspferd::object>("my_obj2");
  BOOST_CHECK(flusspferd::global().has_property("my_obj"));
  BOOST_CHECK(flusspferd::global().has_property("my_obj2"));
  BOOST_CHECK(!flusspferd::global().get_property_object("my_obj").is_null());
  BOOST_CHECK(!flusspferd::global().get_property_object("my_obj2").is_null());
  BOOST_CHECK_NE(
      flusspferd::global().get_property("my_obj"),
      flusspferd::global().get_property("my_obj2"));
}

BOOST_AUTO_TEST_CASE(array) {
  flusspferd::create_on(flusspferd::global())
    .create<flusspferd::array>("name", 12);
  BOOST_CHECK(flusspferd::global().has_property("name"));
  try {
    flusspferd::array arr(flusspferd::global().get_property_object("name"));
    BOOST_CHECK_EQUAL(arr.length(), 12);
  } catch (flusspferd::exception&) {
    BOOST_CHECK(!"No throw!");
  }
}

BOOST_AUTO_TEST_CASE(native_function) {
  flusspferd::create_on(flusspferd::global())
    .create<my_functor>("name", boost::fusion::make_vector(0));
  try {
    flusspferd::function f(flusspferd::global().get_property_object("name"));
    BOOST_CHECK(!f.is_null());
    BOOST_CHECK_EQUAL(f.get_property("called"), flusspferd::value());
    BOOST_CHECK_NO_THROW(f.call(flusspferd::global()));
    BOOST_CHECK_EQUAL(f.get_property("called"), flusspferd::value(false));
  } catch (flusspferd::exception&) {
    BOOST_CHECK(!"No throw!");
  }
}

BOOST_AUTO_TEST_CASE(native_object) {
  flusspferd::create_on(flusspferd::global())
    .create<my_class>("name", boost::fusion::make_vector(5, "hey"));
  try {
    my_class &x = flusspferd::get_native<my_class>(
        flusspferd::global().get_property_object("name"));
    BOOST_CHECK(!x.is_null());
    BOOST_CHECK_EQUAL(x.constructor_choice, my_class::ab);
    BOOST_CHECK_EQUAL(x.a, 5);
    BOOST_CHECK_EQUAL(x.b, "hey");
  } catch (flusspferd::exception&) {
    BOOST_CHECK(!"No throw!");
  }
}

BOOST_AUTO_TEST_CASE(function) {
  flusspferd::create_on(flusspferd::global())
    .create<flusspferd::function>(
        "name",
        "return x + y",
        list_of("x")("y"),
        "file",
        666);
  try {
    flusspferd::function f(flusspferd::global().get_property_object("name"));
    BOOST_CHECK_EQUAL(f.name(), "name");
    BOOST_CHECK_EQUAL(f.arity(), 2);
    flusspferd::root_value v;
    BOOST_CHECK_NO_THROW(v = f.call(flusspferd::global(), 1, 2));
    BOOST_CHECK_EQUAL(v, flusspferd::value(3));
  } catch (flusspferd::exception&) {
    BOOST_CHECK(!"No throw!");
  }
}

BOOST_AUTO_TEST_SUITE_END() // create_on

BOOST_AUTO_TEST_SUITE_END()
