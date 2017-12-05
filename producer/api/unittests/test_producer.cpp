#include <gtest/gtest.h>
#include <producer/producer.h>

namespace {
TEST(VERSION, VersionAboveZero) {
    EXPECT_GE(hidra2::Producer::VERSION, 0);
}

TEST(CreateProducer, PointerIsNotNullptr) {
    hidra2::Producer* prod = hidra2::Producer::CreateProducer("127.0.0.1");
    EXPECT_NE(prod, nullptr);
}
}
