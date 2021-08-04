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
                                                SourceCredentials{asapo::SourceType::kRaw, "", "", "b", "", "", ""}, 3600000, &err);

    EXPECT_THAT(err, Eq(nullptr));
    EXPECT_THAT(dynamic_cast<asapo::ProducerImpl*>(producer.get()), Ne(nullptr));
}

TEST(CreateProducer, ErrorBeamtime) {
    asapo::Error err;
    std::string expected_beamtimeid(asapo::kMaxMessageSize * 10, 'a');
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{asapo::SourceType::kRaw, "instance", "step", expected_beamtimeid, "", "", ""}, 3600000, &err);

    EXPECT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    EXPECT_THAT(producer, Eq(nullptr));
}

TEST(CreateProducer, ErrorOnBothAutoBeamlineBeamtime) {
    asapo::SourceCredentials creds{asapo::SourceType::kRaw, "instance", "step", "auto", "auto", "subname", "token"};
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("endpoint", 4, asapo::RequestHandlerType::kTcp,
                                                creds, 3600000, &err);

    EXPECT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    EXPECT_THAT(producer, Eq(nullptr));
}

TEST(CreateProducer, TooManyThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("", asapo::kMaxProcessingThreads + 1,
                                                asapo::RequestHandlerType::kTcp, SourceCredentials{asapo::SourceType::kRaw, "", "", "", "", "", ""}, 3600000, &err);

    EXPECT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    EXPECT_THAT(producer, Eq(nullptr));
}


TEST(CreateProducer, ZeroThreads) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("", 0,
                                                asapo::RequestHandlerType::kTcp, SourceCredentials{asapo::SourceType::kRaw, "", "", "bt", "", "", ""}, 3600000, &err);

    EXPECT_THAT(err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    EXPECT_THAT(producer, Eq(nullptr));
}


TEST(Producer, SimpleWorkflowWihoutConnection) {
    asapo::Error err;
    std::unique_ptr<asapo::Producer> producer = asapo::Producer::Create("hello", 5, asapo::RequestHandlerType::kTcp,
                                                SourceCredentials{asapo::SourceType::kRaw, "", "", "bt", "", "", ""}, 3600000,
                                                &err);

    EXPECT_THAT(err, Eq(nullptr));
    ASSERT_THAT(producer, Ne(nullptr));

    asapo::MessageHeader message_header{1, 1, "test"};
    auto err_send = producer->Send(message_header, nullptr, asapo::kTransferMetaDataOnly, "stream", nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_THAT(err_send, Eq(nullptr));
}



}
