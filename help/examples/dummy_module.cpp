#include <flusspferd.hpp>
#include <iostream>
#include <ostream>

FLUSSPFERD_LOADER(exports) {
    std::cout << "Module loaded!" << std::endl;
}
