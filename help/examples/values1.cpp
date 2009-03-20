#include <flusspferd/current_context_scope.hpp>
#include <flusspferd/context.hpp>
#include <flusspferd/init.hpp>
#include <flusspferd/value.hpp>
#include <flusspferd/object.hpp>
#include <flusspferd/string.hpp>
#include <flusspferd/local_root_scope.hpp>
#include <cassert>

int main() {
  flusspferd::current_context_scope context_scope(flusspferd::context::create());

  // See below. You can ignore this line for now.
  flusspferd::local_root_scope root_scope;

  // undefined
  flusspferd::value v1;
  assert(v1.is_undefined());

  // boolean (true)
  flusspferd::value v2(true);
  assert(v2.is_boolean());
  assert(v2.get_boolean());

  // integer (1)
  flusspferd::value v3(1);
  assert(v3.is_int());
  assert(v3.is_number());
  assert(v3.get_int() == 1);

  // double (1.1)
  flusspferd::value v4(1.1);
  assert(v4.is_double());
  assert(v4.is_number());
  assert(v4.get_double() == 1.1);

  // object (null)
  flusspferd::object o1;
  assert(o1.is_null());
  flusspferd::value v5(o1);
  assert(v5.is_object());
  assert(v5.is_null());

  // another object (the global object)
  flusspferd::object o2 = flusspferd::global();
  assert(!o2.is_null());

  // string (empty)
  flusspferd::string s; // <- valid (!), empty string
  assert(s.empty());
  flusspferd::value v6(s);
  assert(v6.is_string());
}

