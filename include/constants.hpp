#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cmath> 

namespace Constants {
    const double G = 6.6743e-11; 
    const float C = 299792458.0f;
    const float DEFAULT_INIT_MASS = static_cast<float>(std::pow(10, 22));
    const float DEFAULT_SIZE_RATIO = 30000.0f;
    const float PI = 3.14159265359f;
}

#endif 