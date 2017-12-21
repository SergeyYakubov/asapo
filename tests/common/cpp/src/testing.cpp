#include "testing.h"

#include <iostream>

namespace hidra2 {

template<typename T>
void T_AssertEq(const T& expected, const T& got) {
    if (expected != got) {
        std::cerr << "Assert failed:\n"
                  << "Expected:\t'" << expected << "'\n"
                  << "Obtained:\t'" << got << "'\n";
        exit(EXIT_FAILURE);
    }
}

void M_AssertTrue(bool value) {
    if (!value) {
        std::cerr << "Assert failed:\n"
                  << "Expected:\t'" << "1" << "'\n"
                  << "Obtained:\t'" << value << "'\n";
        exit(EXIT_FAILURE);
    }

}

void M_AssertEq(const std::string& expected, const std::string& got) {
    T_AssertEq(expected, got);
}

void M_AssertEq(int expected, int got) {
    T_AssertEq(expected, got);
}


}

