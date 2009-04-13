#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

int main() {
    flusspferd::current_context_scope context_scope(
        flusspferd::context::create());

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

    // Uneval
    std::cout << x.call("toSource") << std::endl;

    // Check if 'toSource' is enumerable
    // (Note that there is another way to do it (see flusspferd::property::get_attributes),
    // but we call the method here to show how methods can be called.)
    std::cout << x.call("propertyIsEnumerable", "toSource") << std::endl;
}

