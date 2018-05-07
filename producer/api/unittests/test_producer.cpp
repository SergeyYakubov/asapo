#include <gtest/gtest.h>
#include <unittests/MockIO.h>

#include "producer/producer.h"
#include "../src/producer_impl.h"
using ::testing::Ne;

namespace {

TEST(CreateProducer, PointerIsNotNullptr) {
    std::unique_ptr<hidra2::Producer> producer = hidra2::Producer::Create();
    ASSERT_THAT(dynamic_cast<hidra2::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(producer.get(), Ne(nullptr));
}

}
