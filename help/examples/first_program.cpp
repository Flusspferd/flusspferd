#include <flusspferd.hpp>

int main() {
  flusspferd::current_context_scope context_scope(
      flusspferd::context::create());

  // Flusspferd is now initialized. Now we can do what we really want to do:
  // nothing.
}
