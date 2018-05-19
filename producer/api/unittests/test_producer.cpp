#include <gtest/gtest.h>
#include <unittests/MockIO.h>

#include "producer/producer.h"
#include "../src/producer_impl.h"

using ::testing::Ne;
using ::testing::Eq;

namespace {

TEST(CreateProducer, TcpProducer) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint",4,asapo::RequestHandlerType::kTcp, &err);
    ASSERT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
}

TEST(CreateProducer, FileSystemProducer) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4,asapo::RequestHandlerType::kFilesystem, &err);
    ASSERT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
}


TEST(CreateProducer, TooManyThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("", asapo::kMaxProcessingThreads + 1,
                                                                        asapo::RequestHandlerType::kTcp, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Ne(nullptr));
}

TEST(Producer, SimpleWorkflowWihoutConnection) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("hello", 5, asapo::RequestHandlerType::kTcp,&err);
    auto err_send = producer->Send(1, nullptr, 1, "",nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(producer, Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(err_send, Eq(nullptr));
}



}
