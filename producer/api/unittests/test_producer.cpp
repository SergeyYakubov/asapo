#include <gtest/gtest.h>
#include <producer/producer.h>
#include <unittests/MockIO.h>

using ::testing::Ne;

namespace {

TEST(CreateProducer, PointerIsNotNullptr) {
    std::unique_ptr<hidra2::Producer> producer = hidra2::Producer::Create();
    ASSERT_THAT(producer.get(), Ne(nullptr));
}

}
