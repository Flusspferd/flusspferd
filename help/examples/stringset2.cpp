#include <flusspferd.hpp>
#include <iostream>
#include <ostream>
#include <set>
#include <string>

using namespace flusspferd::param;

// Inherit from flusspferd::native_object_base or a class that derives from it.
class StringSet : public flusspferd::native_object_base {
public:
    // StringSet::class_info is used by load_class.
    struct class_info : flusspferd::class_info {
        static char const *constructor_name() {
            return "StringSet";
        }

        static char const *full_name() {
            return "StringSet";
        }

        // Function for creating the class prototype.
        static object create_prototype() {
            // Create a prototype object (with default prototype).
            flusspferd::object proto = flusspferd::create<flusspferd::object>();

            // Add the methods.
            flusspferd::create_on(proto)
                .create<flusspferd::method>("dump", &StringSet::dump)
                .create<flusspferd::method>("add", &StringSet::add)
                .create<flusspferd::method>("delete", &StringSet::delete_)
                .create<flusspferd::method>("toArray", &StringSet::to_array);

            return proto;
        }
    };

    // The only difference here is that you must initialise
    // flusspferd::native_object_base directly.
    StringSet(flusspferd::object const &self, flusspferd::call_context &x)
        : flusspferd::native_object_base(self)
    {
        std::cout << "Creating StringSet" << std::endl;

        for (flusspferd::arguments::iterator it = x.arg.begin();
                it != x.arg.end();
                ++it) {
            data.insert((*it).to_std_string());
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
        flusspferd::root_array result(flusspferd::create<flusspferd::array>());

        for (iterator it = data.begin(); it != data.end(); ++it) {
            result.push(*it);
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

    flusspferd::create<flusspferd::function>("print", &print, _container = flusspferd::global());

    flusspferd::load_class<StringSet>();

    flusspferd::evaluate(
        "var set = new StringSet('b', 'a', 'd');\n"
        "set.add('c');\n"
        "set.dump();\n"
        "set.delete('a'); set.delete('b');\n"
        "print('As Array: ' +  set.toArray().toSource());\n"
    );
}
