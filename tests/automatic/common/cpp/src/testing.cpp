#include "testing.h"

#include <iostream>
#include <algorithm>

namespace asapo {

std::string EraseSpaces(const std::string& str) {
    auto tmp = str;
    auto end_pos = std::remove(tmp.begin(), tmp.end(), ' ');
    tmp.erase(end_pos, tmp.end());
    return tmp;
}

void _M_INTERNAL_PrintComment(const std::string& comment) {
    std::cout << comment << std::endl;
}

void _M_INTERNAL_ThrowError(const std::string& expected, const std::string& got, const std::string& comment) {
    std::cerr << "Assert failed: " << comment << std::endl
              << "Expected:\t'" << expected << "'" << std::endl
              << "Obtained:\t'" << got << "'" << std::endl;
    exit(EXIT_FAILURE);
}

/// Generic equal check
template<typename T>
void T_AssertEq(const T& expected, const T& got, const std::string& comment) {
    _M_INTERNAL_PrintComment(comment);
    if (expected != got) {
        std::cerr << "Assert failed: " << comment << std::endl
                  << "Expected:\t'" << expected << "'" << std::endl
                  << "Obtained:\t'" << got << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void _M_AssertTrue(bool value, const std::string& comment) {
    _M_INTERNAL_PrintComment(comment);
    if (!value) {
        _M_INTERNAL_ThrowError("true", "false", comment);
    }
}

void _M_AssertContains(const std::string& whole, const std::string& sub, const std::string& comment) {
    auto whole_t = EraseSpaces(whole);
    auto sub_t = EraseSpaces(sub);

    _M_INTERNAL_PrintComment(comment);
    if (whole_t.find(sub_t) == std::string::npos) {
        std::cerr << "Assert failed: " << std::endl
                  << "Got (spaces erased):\t'" << whole_t << "'" << std::endl
                  << "Expected contains (spaces erased):\t'" << sub_t << "'" << std::endl;

        exit(EXIT_FAILURE);
    }
}

void _M_AssertEq(const std::string& expected, const std::string& got, const std::string& comment) {
    T_AssertEq(expected, got, comment);
}

void _M_AssertEq(int expected, int got, const std::string& comment) {
    T_AssertEq(expected, got, comment);
}

void _M_AssertEq(const Error& expected, const Error& got, const std::string& comment) {
    T_AssertEq(expected, got, comment);
}

void _M_AssertEq(const decltype(nullptr)&, const Error& got, const std::string& comment) {
    T_AssertEq(Error{}, got, comment);
}

}

