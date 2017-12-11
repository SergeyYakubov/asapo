#include <gtest/gtest.h>
#include <producer/producer.h>

namespace {

TEST(CreateProducer, PointerIsNotNullptr) {
    std::unique_ptr<hidra2::Producer> producer = hidra2::Producer::create();
    EXPECT_NE(producer, nullptr);
    EXPECT_NE(producer.get(), nullptr);
}

}
