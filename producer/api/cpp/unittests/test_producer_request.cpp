#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockLogger.h"
#include "asapo/common/error.h"
#include "asapo/io/io.h"

#include "asapo/producer/common.h"
#include "asapo/producer/producer_error.h"

#include "../src/request_handler_tcp.h"
#include <asapo/common/networking.h>
#include "asapo/io/io_factory.h"

#include "mocking.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::AllOf;
using testing::NiceMock;

using ::testing::InSequence;
using ::testing::HasSubstr;
using ::testing::Sequence;


TEST(ProducerRequest, Constructor) {
    char  expected_file_name[asapo::kMaxMessageSize] = "test_name";
    char  expected_source_credentials[asapo::kMaxMessageSize] = "test_beamtime_id%test_streamid%test_token";
    uint64_t expected_file_id = 42;
    uint64_t expected_file_size = 1337;
    uint64_t expected_meta_size = 137;
    std::string expected_meta = "meta";
    asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;

    asapo::GenericRequestHeader header{expected_op_code, expected_file_id, expected_file_size,
                                       expected_meta_size, expected_file_name};
    asapo::ProducerRequest request{expected_source_credentials, std::move(header), nullptr, expected_meta, "", nullptr, true, 0};

    ASSERT_THAT(request.source_credentials, Eq(expected_source_credentials));
    ASSERT_THAT(request.metadata, Eq(expected_meta));
    ASSERT_THAT(request.header.message, testing::StrEq(expected_file_name));
    ASSERT_THAT(request.header.data_size, Eq(expected_file_size));
    ASSERT_THAT(request.header.data_id, Eq(expected_file_id));
    ASSERT_THAT(request.header.op_code, Eq(expected_op_code));
    ASSERT_THAT(request.header.meta_size, Eq(expected_meta_size));

}


TEST(ProducerRequest, Destructor) {
// fails with data corruption if done wrong
    char data_[100];
    asapo::MessageData data{(uint8_t*)data_};
    asapo::GenericRequestHeader header{asapo::kOpcodeTransferData, 1, 1, 1, ""};
    asapo::ProducerRequest* request = new asapo::ProducerRequest{"", std::move(header), std::move(data), "", "", nullptr, false, 0};

    delete request;


}

}
