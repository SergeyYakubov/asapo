#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "common/error.h"
#include "io/io.h"

#include "producer/common.h"
#include "producer/producer_error.h"

#include "../src/request_handler_filesystem.h"
#include "../src/producer_request.h"

#include "io/io_factory.h"

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


TEST(RequestHandlerFileSystem, Constructor) {
    asapo::RequestHandlerFilesystem request_handler{"destination", 1};

    ASSERT_THAT(dynamic_cast<const asapo::IO*>(request_handler.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(request_handler.log__), Ne(nullptr));
}

class RequestHandlerFilesystemTests : public testing::Test {
  public:
    NiceMock<asapo::MockIO> mock_io;

    uint64_t expected_file_id = 42;
    uint64_t expected_file_size = 1337;
    std::string  expected_file_name = "test_name";
    uint64_t expected_thread_id = 2;
    std::string  expected_destination = "destination";
    std::string expected_fullpath = expected_destination + "/" + expected_file_name;
    std::string expected_origin_fullpath = std::string("origin/") + expected_file_name;

    asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;
    asapo::Error callback_err;
    asapo::GenericRequestHeader header{expected_op_code, expected_file_id, expected_file_size, expected_file_name};
    bool called = false;
    asapo::GenericRequestHeader callback_header;
    asapo::ProducerRequest request{"", header, nullptr, "", [this](asapo::GenericRequestHeader header, asapo::Error err) {
        called = true;
        callback_err = std::move(err);
        callback_header = header;
    }};

    asapo::ProducerRequest request_nocallback{"", header, nullptr, "", nullptr};
    asapo::ProducerRequest request_filesend{"", header, nullptr, expected_origin_fullpath, nullptr};

    testing::NiceMock<asapo::MockLogger> mock_logger;

    asapo::RequestHandlerFilesystem request_handler{expected_destination, expected_thread_id};

    void SetUp() override {
        request_handler.log__ = &mock_logger;
        request_handler.io__.reset(&mock_io);
    }
    void TearDown() override {
        request_handler.io__.release();
    }
};

ACTION_P(A_WriteSendDataResponse, error_code) {
    ((asapo::SendDataResponse*)arg1)->op_code = asapo::kOpcodeTransferData;
    ((asapo::SendDataResponse*)arg1)->error_code = error_code;
}

MATCHER_P2(M_CheckSendDataRequest, file_id, file_size,
           "Checks if a valid GenericRequestHeader was Send") {
    return ((asapo::GenericRequestHeader*)arg)->op_code == asapo::kOpcodeTransferData
           && ((asapo::GenericRequestHeader*)arg)->data_id == file_id
           && ((asapo::GenericRequestHeader*)arg)->data_size == file_size;
}

TEST_F(RequestHandlerFilesystemTests, CallBackErrorIfCannotSaveFile) {
    EXPECT_CALL(mock_io, WriteDataToFile_t(expected_destination, expected_file_name, nullptr, expected_file_size, true))
    .WillOnce(
        Return(
            asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
    );


    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(callback_err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestHandlerFilesystemTests, WorksWithemptyCallback) {
    EXPECT_CALL(mock_io, WriteDataToFile_t(expected_destination, expected_file_name, nullptr, expected_file_size, true))
    .WillOnce(
        Return(nullptr)
    );


    auto err = request_handler.ProcessRequestUnlocked(&request_nocallback);

    ASSERT_THAT(called, Eq(false));
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(RequestHandlerFilesystemTests, FileRequestErrorOnReadData) {

    EXPECT_CALL(mock_io, GetDataFromFile_t(expected_origin_fullpath, testing::Pointee(expected_file_size), _))
    .WillOnce(
        DoAll(
            testing::SetArgPointee<2>(asapo::IOErrorTemplates::kUnknownIOError.Generate().release()),
            Return(nullptr)
        ));

    auto err = request_handler.ProcessRequestUnlocked(&request_filesend);
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(RequestHandlerFilesystemTests, FileRequestOK) {

    EXPECT_CALL(mock_io, GetDataFromFile_t(expected_origin_fullpath, testing::Pointee(expected_file_size), _))
    .WillOnce(
        DoAll(
            testing::SetArgPointee<2>(nullptr),
            Return(nullptr)
        ));

    EXPECT_CALL(mock_io, WriteDataToFile_t(expected_destination, expected_file_name, nullptr, expected_file_size, true))
    .WillOnce(
        Return(nullptr)
    );

    auto err = request_handler.ProcessRequestUnlocked(&request_filesend);
    ASSERT_THAT(err, Eq(nullptr));
}



TEST_F(RequestHandlerFilesystemTests, TransferOK) {
    EXPECT_CALL(mock_io, WriteDataToFile_t(expected_destination, expected_file_name, nullptr, expected_file_size, true))
    .WillOnce(
        Return(
            nullptr)
    );

    request_handler.PrepareProcessingRequestLocked();
    auto err = request_handler.ProcessRequestUnlocked(&request);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(callback_err, Eq(nullptr));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(callback_header.data_size, Eq(header.data_size));
    ASSERT_THAT(callback_header.op_code, Eq(header.op_code));
    ASSERT_THAT(callback_header.data_id, Eq(header.data_id));
    ASSERT_THAT(std::string{callback_header.message}, Eq(std::string{header.message}));
}


}
