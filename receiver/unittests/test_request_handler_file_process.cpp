#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_process.h"
#include "common/networking.h"
#include "mock_receiver_config.h"
#include "preprocessor/definitions.h"

#include "receiver_mocking.h"

using ::testing::Test;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::testing::AllOf;
using ::testing::HasSubstr;


using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;
using asapo::Request;
using asapo::RequestHandlerFileProcess;
using ::asapo::GenericRequestHeader;
using asapo::MockRequest;

namespace {

TEST(FileWrite, Constructor) {
    RequestHandlerFileProcess handler(nullptr);
    ASSERT_THAT(dynamic_cast<asapo::IO*>(handler.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}

class FileWriteHandlerTests : public Test {
  public:
    asapo::MockFileProcessor mock_file_processor;
    RequestHandlerFileProcess handler{&mock_file_processor};
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequest> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    void ExpecFileProcess(const asapo::SimpleErrorTemplate* error_template, bool overwrite);
    void SetUp() override {
        GenericRequestHeader request_header;
        mock_request.reset(new MockRequest{request_header, 1, "", nullptr});
        handler.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        handler.log__ = &mock_logger;
    }
    void TearDown() override {
        handler.io__.release();
    }

};

TEST_F(FileWriteHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kDisk));
}

void FileWriteHandlerTests::ExpecFileProcess(const asapo::SimpleErrorTemplate* error_template, bool overwrite) {
    EXPECT_CALL(mock_file_processor, ProcessFile_t(mock_request.get(), overwrite))
    .WillOnce(
        Return(error_template == nullptr ? nullptr : error_template->Generate().release()));
}

TEST_F(FileWriteHandlerTests, FileAlreadyExists_NoRecordInDb) {
    EXPECT_CALL(*mock_request, SetWarningMessage(HasSubstr("overwritten")));
    EXPECT_CALL(*mock_request, CheckForDuplicates_t())
    .WillOnce(
        Return(nullptr)
    );

    EXPECT_CALL(mock_logger, Warning(HasSubstr("overwriting")));

    ExpecFileProcess(&asapo::IOErrorTemplates::kFileAlreadyExists, false);
    ExpecFileProcess(nullptr, true);

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(FileWriteHandlerTests, FileAlreadyExists_DuplicatedRecordInDb) {

    EXPECT_CALL(*mock_request, SetWarningMessage(HasSubstr("ignore")));
    EXPECT_CALL(*mock_request, SetAlreadyProcessedFlag());
    EXPECT_CALL(mock_logger, Warning(HasSubstr("duplicated")));
    EXPECT_CALL(*mock_request, GetDataID()).WillOnce(Return(1));

    ExpecFileProcess(&asapo::IOErrorTemplates::kFileAlreadyExists, false);

    EXPECT_CALL(*mock_request, CheckForDuplicates_t())
    .WillOnce(
        Return(asapo::ReceiverErrorTemplates::kWarningDuplicatedRequest.Generate().release())
    );

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(FileWriteHandlerTests, FileAlreadyExists_DifferentRecordInDb) {

    ExpecFileProcess(&asapo::IOErrorTemplates::kFileAlreadyExists, false);

    EXPECT_CALL(*mock_request, CheckForDuplicates_t())
    .WillOnce(
        Return(asapo::ReceiverErrorTemplates::kBadRequest.Generate().release())
    );


    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
}



}