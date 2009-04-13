#include <flusspferd.hpp>
#include <iostream>
#include <ostream>
#include <set>
#include <string>

class StringSet : public flusspferd::native_object_base {
public:
  struct class_info : flusspferd::class_info {
    static char const *constructor_name() { return "StringSet"; }
    static char const *full_name() { return "StringSet"; }

    static object create_prototype() {
      flusspferd::object proto = flusspferd::create_object();
      create_native_method(proto, "dump", &StringSet::dump);
      create_native_method(proto, "add", &StringSet::add);
      create_native_method(proto, "delete", &StringSet::delete_);
      create_native_method(proto, "toArray", &StringSet::to_array);
      return proto;
    }
  };

  StringSet(flusspferd::object const &self, flusspferd::call_context &x)
    : flusspferd::native_object_base(self)
  {
    std::cout << "Creating StringSet" << std::endl;

    for (flusspferd::arguments::iterator it = x.arg.begin();
         it != x.arg.end();
         ++it)
    {
       data.insert(flusspferd::string(*it).to_string());
    }
  }

  ~StringSet() {
    std::cout << "Destroying StringSet" << std::endl;
  }

private:
  typedef std::set<std::string> container;
  typedef container::iterator iterator;

  void dump() {
    std::cout << "Dumping StringSet: ";
    for (iterator it = data.begin(); it != data.end(); ++it) {
      if (it != data.begin())
        std::cout << ',';
      std::cout << *it;
    }
    std::cout << std::endl;
  }

  void add(std::string const &x) {
    data.insert(x);
  }

  void delete_(std::string const &x) {
    if (!data.erase(x))
      throw flusspferd::exception("No such element");
  }

  flusspferd::array to_array() {
    flusspferd::array result = flusspferd::create_array();

    for (iterator it = data.begin(); it != data.end(); ++it) {
      result.call("push", *it);
    }

    return result;
  }

private:
  container data;
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
    "var set = new StringSet('b', 'a', 'd');\n"
    "set.add('c');\n"
    "set.dump();\n"
    "set.delete('a'); set.delete('b');\n"
    "print('As Array: ' +  set.toArray().toSource());\n"
  );
}
