#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

extern "C" flusspferd::value flusspferd_load(flusspferd::object container) {
    std::cout << "Module loaded!" << std::endl;
    return flusspferd::string("Here, take this cookie.");
}
