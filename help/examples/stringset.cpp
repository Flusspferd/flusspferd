#include <flusspferd.hpp>
#include <iostream>
#include <ostream>
#include <set>
#include <string>

FLUSSPFERD_CLASS_DESCRIPTION(
    StringSet,
    (full_name, "StringSet")
    (constructor_name, "StringSet")
    (methods,
        ("dump", bind, dump)
        ("add", bind, add)
        ("delete", bind, delete_)
        ("toArray", bind, to_array)))
{
public:
    StringSet(flusspferd::object const &self, flusspferd::call_context &x)
        : base_type(self)
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

public:
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

    flusspferd::create<flusspferd::function>(
        "print", &print,
        flusspferd::param::_container = flusspferd::global());

    flusspferd::load_class<StringSet>();

    flusspferd::evaluate(
        "var set = new StringSet('b', 'a', 'd');\n"
        "set.add('c');\n"
        "set.dump();\n"
        "set.delete('a'); set.delete('b');\n"
        "print('As Array: ' +  set.toArray().toSource());\n"
    );
}
