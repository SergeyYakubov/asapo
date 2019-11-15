#include <gtest/gtest.h>
#include <unittests/MockIO.h>

#include "producer/producer.h"
#include "../src/producer_impl.h"
#include "producer/producer_error.h"

using ::testing::Ne;
using ::testing::Eq;

using asapo::SourceCredentials;

namespace {

TEST(CreateProducer, TcpProducer) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{"bt", "", ""}, &err);
    ASSERT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
}

TEST(CreateProducer, ErrorBeamtime) {
    asapo::Error err;
    std::string expected_beamtimeid(asapo::kMaxMessageSize * 10, 'a');
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{expected_beamtimeid, "", ""}, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}


//todo: memtest fails on old linux machine. Add valgrind suppressions?
/*
TEST(CreateProducer, FileSystemProducer) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4,
                                                asapo::RequestHandlerType::kFilesystem, "bt", &err);
    ASSERT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
}
*/

TEST(CreateProducer, TooManyThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("", asapo::kMaxProcessingThreads + 1,
                                                asapo::RequestHandlerType::kTcp, SourceCredentials{"bt", "", ""}, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}


TEST(CreateProducer, ZeroThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("", 0,
                                                asapo::RequestHandlerType::kTcp, SourceCredentials{"bt", "", ""}, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}


TEST(Producer, SimpleWorkflowWihoutConnection) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("hello", 5, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{"bt", "", ""},
                                                &err);

    asapo::EventHeader event_header{1, 1, "test"};
    auto err_send = producer->SendData(event_header, nullptr, asapo::kTransferMetaDataOnly, nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(producer, Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(err_send, Eq(nullptr));
}



}
