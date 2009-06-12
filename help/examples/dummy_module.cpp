#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

FLUSSPFERD_LOADER_SIMPLE(exports) {
    std::cout << "Module loaded!" << std::endl;
}
