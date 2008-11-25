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

#include "flusspferd/io/file_class.hpp"
#include "flusspferd/class.hpp"
#include "flusspferd/create.hpp"
#include "flusspferd/function_adapter.hpp"
#include "flusspferd/native_function.hpp"
#include "flusspferd/convert.hpp"
#include "flusspferd/native_function_base.hpp"
#include "flusspferd/native_object_base.hpp"
#include "flusspferd/call_context.hpp"
#include "flusspferd/init.hpp"
#include "flusspferd/root.hpp"
#include "flusspferd/context.hpp"
#include "flusspferd/value_io.hpp"
#include "flusspferd/evaluate.hpp"
#include "flusspferd/arguments.hpp"
#include "flusspferd/current_context_scope.hpp"
#include "flusspferd/exception.hpp"
#include "flusspferd/tracer.hpp"
#include "flusspferd/property_iterator.hpp"
#include <iostream>
#include <stdexcept>

struct my_object : flusspferd::native_object_base {
  my_object(flusspferd::call_context &x)
    : test(x.arg[0].to_string().to_string())
  {
    std::cout << "my_object construct" << std::endl;
  }

  ~my_object() {
    std::cout << "my_object destruct" << std::endl;
  }

  std::string test;
  flusspferd::value v;

  void post_initialize() {
    std::cout << "my_object pi" << std::endl;

    register_native_method("foo", &my_object::foo);
    register_native_method("()", 0);

    v = flusspferd::string(test);
  }

  void property_add(flusspferd::value const &id, flusspferd::value &v) {
    std::cout << "my_object add " << id << " = " << v << std::endl;
  }

  void property_get(flusspferd::value const &id, flusspferd::value &v) {
    std::cout << "my_object get " << id << " = " << v << std::endl;
  }

  void property_set(flusspferd::value const &id, flusspferd::value &v) {
    std::cout << "my_object set " << id << " = " << v << std::endl;
  }

  bool property_delete(flusspferd::value const &id) {
    std::cout << "my_object delete " << id << std::endl;
    return true;
  }

  void foo(int i) {
    std::cout << "my_object foo " << i << std::endl;
  }

  void call_native_method(std::string const &name, flusspferd::call_context &x)
  {
    std::cout << "my_object call method " << name << " v:" << v << std::endl;
    flusspferd::native_object_base::call_native_method(name, x);
  }

  void trace(flusspferd::tracer &trc) {
    trc("v", v);
  }

  struct class_info : flusspferd::class_info {
    static object create_prototype() {
      flusspferd::object proto = flusspferd::create_object();
      flusspferd::root_value rv(proto);

      proto.define_property(
        "foo",
        flusspferd::native_object_base::create_native_method("foo", 1U),
        flusspferd::object::dont_enumerate |
        flusspferd::object::read_only_property);

      return proto;
    }

    static std::size_t constructor_arity() { return 1; }
    static char const *constructor_name() { return "MyObject"; }
  };
};

struct my_function : flusspferd::native_function_base {
  my_function(double x = 4.2) : first(true), x(x) {}

  bool first;
  double x;

  void call(flusspferd::call_context &x) {
    std::cout << "Function called!" << std::endl;
    if (first) {
      first = false;
      flusspferd::root_value v(flusspferd::value(std::string("testtest")));
      std::vector<flusspferd::value> arg(1, v);
      x.function.call(arg);
    }
    std::cout << "[\n";
    typedef flusspferd::arguments::iterator iterator;
    iterator const end = x.arg.end();
    for(iterator i = x.arg.begin(); i != end; ++i)
      std::cout << "Arg: " << *i << '\n';
    std::cout << "]\n";
    x.result = flusspferd::value(this->x);
  }

  ~my_function() { std::cout << "my_function destructor" << std::endl; }
};

flusspferd::string function2(flusspferd::object &x, int) {
  return flusspferd::value(x).to_string();
}

int main() {
  //try {
    flusspferd::init::initialize();

    flusspferd::context co = flusspferd::context::create();
    flusspferd::current_context_scope scope(co);

    flusspferd::load_class<flusspferd::io::file_class>();

    flusspferd::value num(-1234567891234.5678);
    std::cout.precision(40);
    std::cout << num.to_number() << '\n';
    std::cout << num.to_integral_number(16, false) << '\n';
    std::cout << num.to_integral_number(16, true) << '\n';
    std::cout << num.to_integral_number(32, false) << '\n';
    std::cout << num.to_integral_number(32, true) << '\n';
    std::cout << num.to_integral_number(64, false) << '\n';
    std::cout << num.to_integral_number(64, true) << '\n' << std::endl;

    std::cout << 
      flusspferd::convert<long long>::from_value().perform(num) << '\n';
    std::cout <<
      flusspferd::convert<unsigned short const &>::from_value().perform(num);
    std::cout << '\n' << std::endl;

    //throw flusspferd::exception("bling");

    {
      //flusspferd::function mk =
      flusspferd::load_class<my_object>();

      //flusspferd::root_value v(mk.call());
      flusspferd::root_object o(
        flusspferd::global().call("MyObject").to_object());
      co.gc();
      o.call("foo");
      co.gc();

      flusspferd::value v(o);

      flusspferd::convert<flusspferd::native_object_base *>::from_value
          from_value;
      flusspferd::native_object_base *p = from_value.perform(v);

      flusspferd::convert<flusspferd::native_object_base const &>::to_value
          to_value;
      o = to_value.perform(*p).get_object();

      o.call();
      co.gc();

      o.set_property("zzz", flusspferd::value(4));
      o.set_property("zzz", flusspferd::value(5));
      o.delete_property("zzz");
    }

    co.gc();
    
    char const *source0 = "var hw = 'Hello World\\n';";
    flusspferd::evaluate(source0, __FILE__, 0);
  
    char const *source1 = "hw";
    flusspferd::root_value v(flusspferd::evaluate(source1, __FILE__, 1));

    std::cout << v << '\n';

    flusspferd::function x =
      flusspferd::create_native_function<my_function>(4.3);

    flusspferd::global().set_property("fun", x);

    char const *source2 = "fun(1.2, \"hallo\")";
    v = flusspferd::evaluate(source2, __FILE__, 2);

    std::cout << v << "\n\n";

    flusspferd::global().call("fun");

    flusspferd::global().define_property(
      "abc", flusspferd::value(), flusspferd::object::read_only_property, x);

    char const *source3 = "abc";
    v = flusspferd::evaluate(source3, __FILE__, 3);

    std::cout << v << "\n\n" << std::flush;

    char const *source4 = "abc = 5; fun(abc)";
    flusspferd::evaluate(source4, __FILE__, 4);

    flusspferd::object glob = flusspferd::global();
    for (flusspferd::property_iterator it = glob.begin();
        it != glob.end();
        ++it)
    {
      std::cout << "glob: " << *it << std::endl;
    }

    flusspferd::global().delete_property("fun");
    flusspferd::global().delete_property("abc");

    co.gc();

    std::cout << "---------" << std::endl;

    flusspferd::root_function f_x(flusspferd::create_native_function(
          &function2, std::string("function2")));

    std::cout << "function2: " << f_x.call() << std::endl;
    std::cout << "name/arity: " <<
        f_x.name() << '/' << f_x.arity() << std::endl;
  //}
  //catch(std::exception &e) {
  //  std::cerr << "Exception: " << e.what() << '\n';
  //  return 1;
  //}
  return 0;
}
