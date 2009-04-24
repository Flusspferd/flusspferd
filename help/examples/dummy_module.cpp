#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

extern "C" void flusspferd_load(flusspferd::object container) {
    std::cout << "Module loaded!" << std::endl;
}
