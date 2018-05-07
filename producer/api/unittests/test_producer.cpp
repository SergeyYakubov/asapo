#include <gtest/gtest.h>
#include <unittests/MockIO.h>

#include "producer/producer.h"
#include "../src/producer_impl.h"
using ::testing::Ne;

namespace {

TEST(CreateProducer, PointerIsNotNullptr) {
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create();
    ASSERT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(producer.get(), Ne(nullptr));
}

}
