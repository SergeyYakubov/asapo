#include "testing.h"

#include <iostream>

namespace hidra2 {

template<typename T>
void T_AssertEq(const T& expected, const T& got) {
    if (expected != got) {
        std::cerr << "Assert failed:\n"
                  << "Expected:\t'" << expected << "'\n"
                  << "Obtained:\t'" << got << "'\n";
        abort();
    }
}


void M_AssertEq(const std::string& expected, const std::string& got) {
    T_AssertEq(expected, got);
}

void M_AssertEq(int expected, int got) {
    T_AssertEq(expected, got);
}


}

