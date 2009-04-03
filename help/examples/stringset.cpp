#include <flusspferd.hpp>
#include <iostream>
#include <ostream>
#include <set>

class StringSet : public flusspferd::native_object_base {
public:
  struct class_info : flusspferd::class_info {
    static char const *constructor_name() { return "StringSet"; }
    static char const *full_name() { return "StringSet"; }

    static object create_prototype() {
      flusspferd::object proto = flusspferd::create_object();
      create_native_method(proto, "dump", 0);
      create_native_method(proto, "add", 1);
      create_native_method(proto, "delete", 1);
      create_native_method(proto, "toArray", 0);
      return proto;
    }
  };

  void init() {
    register_native_method("dump", &StringSet::dump);
    register_native_method("add", &StringSet::add);
    register_native_method("delete", &StringSet::delete_);
    register_native_method("toArray", &StringSet::to_array);
  }

  StringSet(flusspferd::object const &self, flusspferd::call_context &x)
    : flusspferd::native_object_base(self)
  {
    init();

    std::cout << "Creating StringSet" << std::endl;

    for (flusspferd::arguments::iterator it = x.arg.begin();
         it != x.arg.end();
         ++it)
    {
       data.insert(flusspferd::string(*it).to_string());
    }
  }

  ~StringSet()
  {
    std::cout << "Destroying StringSet" << std::endl;
  }

private:
  void dump() {
    std::cout << "Dumping StringSet: ";
    for (std::set<std::string>::iterator it = data.begin();
         it != data.end();
         ++it)
    {
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
    data.erase(x);
  }

  flusspferd::array to_array() {
    flusspferd::array result = flusspferd::create_array();

    for (std::set<std::string>::iterator it = data.begin();
         it != data.end();
         ++it)
    {
      result.call("push", *it);
    }

    return result;
  }

private:
  std::set<std::string> data;
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
