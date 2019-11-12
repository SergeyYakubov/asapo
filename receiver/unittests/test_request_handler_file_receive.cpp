#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_receive.h"
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
using asapo::RequestHandlerFileReceive;
using ::asapo::GenericRequestHeader;
using asapo::MockRequest;

namespace {

TEST(FileReceive, Constructor) {
    RequestHandlerFileReceive handler;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(handler.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}

class FileReceiveHandlerTests : public Test {
  public:
    RequestHandlerFileReceive handler;
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequest> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    SocketDescriptor expected_socket_id = SocketDescriptor{1};
    std::string expected_file_name = "2";
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_beamline = "beamline";
    uint64_t expected_file_size = 10;
    void MockRequestData();
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        mock_request.reset(new MockRequest{request_header, expected_socket_id, ""});
        handler.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        handler.log__ = &mock_logger;
    }
    void TearDown() override {
        handler.io__.release();
    }

};

TEST_F(FileReceiveHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(asapo::StatisticEntity::kDisk));
}

void FileReceiveHandlerTests::MockRequestData() {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    EXPECT_CALL(*mock_request, GetSocket())
    .WillOnce(Return(expected_socket_id))
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

TEST_F(FileReceiveHandlerTests, CallsReceiveFile) {
    asapo::ReceiverConfig test_config;
    test_config.root_folder = "test_folder";

    asapo::SetReceiverConfig(test_config, "none");

    MockRequestData();

    std::string expected_path = std::string("test_folder") + asapo::kPathSeparator + expected_beamline
                                + asapo::kPathSeparator + expected_beamtime_id;

    EXPECT_CALL(mock_io, ReceiveDataToFile_t(expected_socket_id, expected_path, expected_file_name, expected_file_size,
                                             true))
    .WillOnce(
        Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
    );

    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}


TEST_F(FileReceiveHandlerTests, WritesToLog) {

    MockRequestData();

    EXPECT_CALL(mock_io, ReceiveDataToFile_t(_, _, _, _, _))
    .WillOnce(Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("received file"),
                                         HasSubstr(expected_file_name),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(std::to_string(expected_file_size))
                                        )
                                  )
               );
    handler.ProcessRequest(mock_request.get());
}


}