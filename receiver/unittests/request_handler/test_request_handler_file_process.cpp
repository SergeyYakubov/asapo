#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_file_process.h"
#include "asapo/common/networking.h"
#include "../mock_receiver_config.h"
#include "asapo/preprocessor/definitions.h"

#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;

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
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    void ExpecFileProcess(const asapo::ErrorTemplateInterface* error_template, bool overwrite);
    void SetUp() override {
        GenericRequestHeader request_header;
        mock_request.reset(new NiceMock<MockRequest>{request_header, 1, "", nullptr});
        handler.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        handler.log__ = &mock_logger;
        SetDefaultRequestCalls(mock_request.get(),"");
    }
    void TearDown() override {
        handler.io__.release();
    }

};

TEST_F(FileWriteHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kDisk));
}

void FileWriteHandlerTests::ExpecFileProcess(const asapo::ErrorTemplateInterface* error_template, bool overwrite) {
    EXPECT_CALL(mock_file_processor, ProcessFile_t(mock_request.get(), overwrite))
    .WillOnce(
        Return(error_template == nullptr ? nullptr : error_template->Generate().release()));
}

TEST_F(FileWriteHandlerTests, FileAlreadyExists_NoRecordInDb) {
    EXPECT_CALL(*mock_request, SetResponseMessage(HasSubstr("overwritten"), asapo::ResponseMessageType::kWarning));
    EXPECT_CALL(*mock_request, CheckForDuplicates_t())
    .WillOnce(
        Return(nullptr)
    );
    std::string ref_str;
    EXPECT_CALL(*mock_request, GetOfflinePath()).WillRepeatedly
    (ReturnRef(ref_str));

    EXPECT_CALL(*mock_request, GetFileName()).WillRepeatedly
    (Return(""));


    EXPECT_CALL(mock_logger, Warning(HasSubstr("overwritting")));

    ExpecFileProcess(&asapo::IOErrorTemplates::kFileAlreadyExists, false);
    ExpecFileProcess(nullptr, true);

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(FileWriteHandlerTests, FileAlreadyExists_DuplicatedRecordInDb) {

    EXPECT_CALL(*mock_request, SetResponseMessage(HasSubstr("ignore"), asapo::ResponseMessageType::kWarning));
    EXPECT_CALL(*mock_request, SetAlreadyProcessedFlag());
    EXPECT_CALL(mock_logger, Warning(HasSubstr("duplicated")));
    EXPECT_CALL(*mock_request, GetDataID()).WillRepeatedly(Return(1));

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
