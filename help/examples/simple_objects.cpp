#include <flusspferd/current_context_scope.hpp>
#include <flusspferd/context.hpp>
#include <flusspferd/init.hpp>
#include <flusspferd/root.hpp>
#include <flusspferd/value_io.hpp>
#include <flusspferd/create.hpp>
#include <flusspferd/property_iterator.hpp>
#include <iostream>
#include <ostream>

int main() {
  flusspferd::current_context_scope context_scope(flusspferd::context::create());

  // Create a simple object
  flusspferd::root_object x(flusspferd::create_object());

  // Set a property
  x.set_property("property", flusspferd::value(1234));

  // Add another - same value
  flusspferd::value tmp = x.get_property("property");
  x.set_property("property2", tmp);

  // Iterate
  for (flusspferd::property_iterator it = x.begin(); it != x.end(); ++it) {
    if (x.has_own_property(*it)) {
      std::cout << *it << ": " << x.get_property(*it) << std::endl;
    }
  }
}

