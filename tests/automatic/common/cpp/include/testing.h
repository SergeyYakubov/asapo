#ifndef HIDRA2_TESTING_H
#define HIDRA2_TESTING_H

#include <string>

namespace hidra2 {

void M_AssertEq(const std::string& expected, const std::string& got);
void M_AssertEq(int expected, int got);
void M_AssertTrue(bool value);
void M_AssertContains(const std::string& whole, const std::string& sub);


}

#endif //HIDRA2_TESTING_H
