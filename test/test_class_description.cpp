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

#include "test_environment.hpp"
#include "flusspferd/class_description.hpp"
#include "flusspferd/create/native_object.hpp"
#include "flusspferd/value_io.hpp"

struct my_functor {
  int operator() () const {
    return 666;
  }
};

FLUSSPFERD_CLASS_DESCRIPTION(
    my_class,
    (constructor_name, "MyClass")
    (full_name, "MyClass")
    (
        methods,
        ("methods_function", function, (
            int (),
            my_functor()
        ))
    )
)
{
public:
  my_class(object const &o)
    : base_type(o)
    {
    }
}; 

BOOST_FIXTURE_TEST_SUITE(
    with_context, context_fixture)

BOOST_AUTO_TEST_CASE(methods_function)
{
  flusspferd::root_object obj(flusspferd::create<my_class>());
  BOOST_CHECK(!obj.is_null());
  BOOST_CHECK(obj.has_property("methods_function"));// FIXME why does this fail?
  BOOST_CHECK_EQUAL(obj.call("methods_function"), flusspferd::value(666));
}

BOOST_AUTO_TEST_SUITE_END()
