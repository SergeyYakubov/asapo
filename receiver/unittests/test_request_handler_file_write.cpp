#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_write.h"
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
using asapo::RequestHandlerFileWrite;
using ::asapo::GenericRequestHeader;
using asapo::MockRequest;

namespace {

TEST(FileWrite, Constructor) {
    RequestHandlerFileWrite handler;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(handler.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}

class FileWriteHandlerTests : public Test {
  public:
    RequestHandlerFileWrite handler;
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequest> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_file_name = "2.bin";
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_beamline = "beamline";
    uint64_t expected_file_size = 10;
    void MockRequestData();
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        mock_request.reset(new MockRequest{request_header, 1, ""});
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


TEST_F(FileWriteHandlerTests, ErrorWhenZeroFileSize) {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(0))
    ;

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
}

TEST_F(FileWriteHandlerTests, ErrorWhenTooBigFileSize) {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(asapo::kMaxFileSize + 1))
    ;

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
}

void FileWriteHandlerTests::MockRequestData() {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    asapo::FileData data;
    EXPECT_CALL(*mock_request, GetData())
    .WillOnce(ReturnRef(data))
    ;

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetBeamline())
    .WillOnce(ReturnRef(expected_beamline))
    ;


    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;
}

TEST_F(FileWriteHandlerTests, CallsWriteFile) {
    asapo::ReceiverConfig test_config;
    test_config.root_folder = "test_folder";

    asapo::SetReceiverConfig(test_config);

    MockRequestData();

    std::string expected_path = std::string("test_folder") + asapo::kPathSeparator + expected_beamline
                                + asapo::kPathSeparator + expected_beamtime_id
                                + asapo::kPathSeparator + expected_file_name;

    EXPECT_CALL(mock_io, WriteDataToFile_t(expected_path.c_str(), _, expected_file_size))
    .WillOnce(
        Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
    );

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}


TEST_F(FileWriteHandlerTests, WritesToLog) {

    MockRequestData();

    EXPECT_CALL(mock_io, WriteDataToFile_t(_, _, _))
    .WillOnce(Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("saved file"),
                                         HasSubstr(expected_file_name),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(std::to_string(expected_file_size))
                                        )
                                  )
               );
    handler.ProcessRequest(mock_request.get());
}


}