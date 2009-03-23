#include <flusspferd/current_context_scope.hpp>
#include <flusspferd/context.hpp>
#include <flusspferd/init.hpp>
#include <flusspferd/value.hpp>
#include <flusspferd/object.hpp>
#include <flusspferd/string.hpp>
#include <flusspferd/string_io.hpp>
#include <flusspferd/value_io.hpp>
#include <flusspferd/create.hpp>
#include <flusspferd/evaluate.hpp>
#include <flusspferd/native_object_base.hpp>
#include <flusspferd/class.hpp>
#include <iostream>
#include <ostream>
#include <set>

class StringSet : public flusspferd::native_object_base {
public:
  struct class_info : flusspferd::class_info {
    static char const *constructor_name() { return "StringSet"; }
    static char const *full_name() { return "StringSet"; }
  };

  StringSet(flusspferd::object const &self, flusspferd::call_context &x)
    : flusspferd::native_object_base(self)
  {
    std::cout << "Creating StringSet" << std::endl;
  }

  ~StringSet()
  {
    std::cout << "Destroying StringSet" << std::endl;
  }
};

void print(flusspferd::string const &x) {
  std::cout << x << std::endl;
}

int main() {
  flusspferd::current_context_scope context_scope(
      flusspferd::context::create());

  flusspferd::create_native_function(flusspferd::global(), "print", &print);

  flusspferd::load_class<StringSet>();

  flusspferd::evaluate(
    "var set = new StringSet('a', 'b', 'c');\n"
    "//set.add('d');\n");
}
