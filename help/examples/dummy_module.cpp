#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

void do_nothing_useful() {
    std::cout << "Yup, this is a function that does nothing useful." << std::endl;
}

FLUSSPFERD_LOADER_SIMPLE(exports) {
    flusspferd::create_native_function(exports, "doNothingUseful", &do_nothing_useful);
    exports.set_property("variable", value(true));
    std::cout << "Module loaded!" << std::endl;
}
