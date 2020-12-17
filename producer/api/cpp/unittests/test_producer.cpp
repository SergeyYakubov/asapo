#include <gtest/gtest.h>
#include "asapo/unittests/MockIO.h"

#include "asapo/producer/producer.h"
#include "../src/producer_impl.h"
#include "asapo/producer/producer_error.h"

using ::testing::Ne;
using ::testing::Eq;

using asapo::SourceCredentials;

namespace {

TEST(CreateProducer, TcpProducer) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{asapo::SourceType::kRaw,"bt", "", "", ""}, 3600, &err);
    ASSERT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
}

TEST(CreateProducer, ErrorBeamtime) {
    asapo::Error err;
    std::string expected_beamtimeid(asapo::kMaxMessageSize * 10, 'a');
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{asapo::SourceType::kRaw,expected_beamtimeid, "", "", ""}, 3600, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST(CreateProducer, ErrorOnBothAutoBeamlineBeamtime) {
    asapo::SourceCredentials creds{asapo::SourceType::kRaw,"auto", "auto", "subname", "token"};
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4, asapo::RequestHandlerType::kTcp,
                                                creds, 3600, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}

TEST(CreateProducer, TooManyThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("", asapo::kMaxProcessingThreads + 1,
                                                asapo::RequestHandlerType::kTcp, SourceCredentials{asapo::SourceType::kRaw,"bt", "", "", ""}, 3600, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}


TEST(CreateProducer, ZeroThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("", 0,
                                                asapo::RequestHandlerType::kTcp, SourceCredentials{asapo::SourceType::kRaw,"bt", "", "", ""}, 3600, &err);
    ASSERT_THAT(producer, Eq(nullptr));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
}


TEST(Producer, SimpleWorkflowWihoutConnection) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("hello", 5, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{asapo::SourceType::kRaw,"bt", "", "", ""}, 3600,
                                                &err);

    asapo::EventHeader event_header{1, 1, "test"};
    auto err_send = producer->SendData(event_header, nullptr, asapo::kTransferMetaDataOnly, nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(producer, Ne(nullptr));
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(err_send, Eq(nullptr));
}



}
