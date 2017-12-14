#include "testing.h"

#include <iostream>

namespace hidra2 {

void M_AssertEq(const std::string& expected, const std::string& got) {
    if (expected != got) {
        std::cerr << "Assert failed:\n"
                  << "Expected:\t'" << expected << "'\n"
                  << "Obtained:\t'" << got << "'\n";
        abort();
    }
}
}

