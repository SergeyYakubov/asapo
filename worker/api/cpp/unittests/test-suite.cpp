/* 
 * File:   TestSuite
 * Author: yakubov
 *
 * Created on Nov 22, 2017, 4:44:43 PM
 */

#include <gtest/gtest.h>

class TestWorkerAPI : public testing::Test {
protected:

    void SetUp() {
        // Setup ...
    }

    void TearDown() {
        // Teardown ...
    }

};

TEST_F(TestWorkerAPI, testConnect) {
    FAIL();
}

