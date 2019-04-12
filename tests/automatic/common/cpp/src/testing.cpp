#include "testing.h"

#include <iostream>
#include <algorithm>

namespace asapo {

template<typename T>
void T_AssertEq(const T& expected, const T& got) {
    if (expected != got) {
        std::cerr << "Assert failed:\n"
                  << "Expected:\t'" << expected << "'\n"
                  << "Obtained:\t'" << got << "'\n";
        exit(EXIT_FAILURE);
    }
}

void M_AssertTrue(bool value, std::string name) {
    if (!value) {
        std::cerr << "Assert failed: " << name << "\n"
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


std::string EraseSpaces(const std::string& str) {
    auto tmp = str;
    auto end_pos = std::remove(tmp.begin(), tmp.end(), ' ');
    tmp.erase(end_pos, tmp.end());
    return tmp;
}
void M_AssertContains( const std::string& whole, const std::string& sub) {
    auto whole_t = EraseSpaces(whole);
    auto sub_t = EraseSpaces(sub);

    if (whole_t.find(sub_t) == std::string::npos) {
        std::cerr << "Assert failed:\n"
                  << "Got (spaces erased):\t'" << whole_t << "'\n"
                  << "Expected containes (spaces erased):\t'" << sub_t << "'\n";

        exit(EXIT_FAILURE);
    }
}


}

