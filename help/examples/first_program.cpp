#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

int main() {
    // Create and use a context, needed to be able to use Flusspferd.
    flusspferd::current_context_scope context_scope(
        flusspferd::context::create());

    // Create a new string and protect it from the garbage collector by making
    // it a root string.
    flusspferd::root_string str("Hello, World!\n");

    // Demonstrate that str will not be garbage collected, by calling the
    // garbage collector.
    // Note that there is no other reason to call the garbage collector here.
    flusspferd::gc();

    // Set a property on the global object. Effectively, this creates a global
    // variable "str" with the contents of str.
    flusspferd::global().set_property("str", str);

    // Print str on std::cout - but first, replace "World" by "Flusspferd".
    std::cout << flusspferd::evaluate("str.replace('World', 'Flusspferd')");
}
