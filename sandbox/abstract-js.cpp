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

#include "flusspferd/convert.hpp"
#include "flusspferd/native_function_base.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/call_context.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/root_value.hpp"
#include "flusspferd/context.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/tracer.hpp"
#include <iostream>
#include <stdexcept>

namespace js = flusspferd::js;

struct my_object : js::native_object_base {
  my_object() { std::cout << "my_object construct" << std::endl; }
  ~my_object() { std::cout << "my_object destruct" << std::endl; }

  js::value v;

  void post_initialize() {
    std::cout << "my_object pi" << std::endl;
    add_native_method("foo", 0U, &my_object::foo);
    add_native_method("()", 0U, 0);
    v = js::string("test");
  }

  void foo(js::call_context &) {
    std::cout << "my_object foo" << std::endl;
  }

  void call_native_method(std::string const &name, js::call_context &x) {
    std::cout << "my_object call method " << name << " v:" << v << std::endl;
    js::native_object_base::call_native_method(name, x);
  }

  void trace(js::tracer &trc) {
    trc("v", v);
  }
};

struct my_function : js::native_function_base {
  my_function() : first(true) {}

  bool first;

  void call(js::call_context &x) {
    std::cout << "Function called!" << std::endl;
    if (first) {
      first = false;
      js::root_value v(js::value(std::string("testtest")));
      std::vector<js::value> arg(1, v);
      x.function.call(arg);
    }
    std::cout << "[\n";
    typedef js::arguments::iterator iterator;
    iterator const end = x.arg.end();
    for(iterator i = x.arg.begin(); i != end; ++i)
      std::cout << "Arg: " << *i << '\n';
    std::cout << "]\n";
    x.result = js::value(4.2);
  }

  ~my_function() { std::cout << "my_function destructor" << std::endl; }
};

struct function2 : js::native_function_base {
  void call(js::call_context &) {
    throw std::runtime_error("boom");
  }
};

int main() {
  //try {
    js::init::initialize();

    js::context co = js::context::create();
    js::current_context_scope scope(co);

    js::value num(-1234567891234.5678);
    std::cout.precision(40);
    std::cout << num.to_number() << '\n';
    std::cout << num.to_integral_number(16, false) << '\n';
    std::cout << num.to_integral_number(16, true) << '\n';
    std::cout << num.to_integral_number(32, false) << '\n';
    std::cout << num.to_integral_number(32, true) << '\n';
    std::cout << num.to_integral_number(64, false) << '\n';
    std::cout << num.to_integral_number(64, true) << '\n' << std::endl;

    std::cout << js::convert<long long>::from_value(num) << '\n';
    std::cout << js::convert<unsigned short>::from_value(num) << '\n' << std::endl;

    //throw js::exception("bling");

    {
      js::root_value v(js::object::create_native(new my_object));
      js::object o = v.get_object();
      co.gc();
      o.call("foo");
      co.gc();
      o.call();
      co.gc();
    }

    co.gc();
    
    char const *source0 = "var hw = 'Hello World\\n';";
    js::evaluate(source0, __FILE__, 0);
  
    char const *source1 = "hw";
    js::root_value v(js::evaluate(source1, __FILE__, 1));

    std::cout << v << '\n';

    js::function x = js::function::create_native(new my_function);

    js::global().set_property("fun", x);

    char const *source2 = "fun(1.2, \"hallo\")";
    v = js::evaluate(source2, __FILE__, 2);

    std::cout << v << "\n\n";

    js::global().call("fun");

    js::global().define_property("abc", js::value(), js::object::read_only_property, x);

    char const *source3 = "abc";
    v = js::evaluate(source3, __FILE__, 3);

    std::cout << v << "\n\n" << std::flush;

    char const *source4 = "abc = 5; fun(abc)";
    js::evaluate(source4, __FILE__, 4);

    js::global().delete_property("fun");
    js::global().delete_property("abc");

    co.gc();

    std::cout << "---------" << std::endl;

    x = js::function::create_native(new function2);
    js::root_value f_x(x);

    x.call();
  //}
  //catch(std::exception &e) {
  //  std::cerr << "Exception: " << e.what() << '\n';
  //  return 1;
  //}
  return 0;
}
