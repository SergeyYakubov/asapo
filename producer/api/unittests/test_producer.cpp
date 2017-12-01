#include <gtest/gtest.h>
#include <producer/producer.h>

namespace {
TEST(VERSION, VersionAboveZero) {
    EXPECT_GE(HIDRA2::Producer::VERSION, 0);
}

TEST(CreateProducer, PointerIsNotNullptr) {
    HIDRA2::Producer* prod = HIDRA2::Producer::CreateProducer("127.0.0.1");
    EXPECT_NE(prod, nullptr);
}
}
