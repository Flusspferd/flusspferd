#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

void do_nothing_useful() {
    std::cout << "Yup, this is a function that does nothing useful." << std::endl;
}

FLUSSPFERD_LOADER_SIMPLE(exports) {
    flusspferd::create<flusspferd::function>(
        "doNothingUseful", &do_nothing_useful,
        flusspferd::param::_container = exports);
    exports.set_property("variable", true);
    std::cout << "Module loaded!" << std::endl;
}
