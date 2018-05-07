#ifndef ASAPO_TESTING_H
#define ASAPO_TESTING_H

#include <string>

namespace asapo {

void M_AssertEq(const std::string& expected, const std::string& got);
void M_AssertEq(int expected, int got);
void M_AssertTrue(bool value);
void M_AssertContains(const std::string& whole, const std::string& sub);


}

#endif //ASAPO_TESTING_H
