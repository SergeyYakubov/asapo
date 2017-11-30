#include <gtest/gtest.h>
#include <common/networking.h>

namespace {

    TEST(Networking, CheckIfNoErrorIsNull) {
        /*
         * To ensure that developers can use
         * if(!response->error_code)
         */
        EXPECT_EQ((uint16_t) HIDRA2::NET_ERR__NO_ERROR, 0);
    }
}
