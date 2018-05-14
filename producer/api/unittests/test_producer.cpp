#include <gtest/gtest.h>
#include <unittests/MockIO.h>

#include "producer/producer.h"
#include "../src/producer_impl.h"
using ::testing::Ne;
using ::testing::Eq;

namespace {

TEST(CreateProducer, PointerIsNotNullptr) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create(4, &err);
    ASSERT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));

}

TEST(CreateProducer, TooManyThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create(asapo::kMaxProcessingThreads + 1, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Ne(nullptr));
}




}
