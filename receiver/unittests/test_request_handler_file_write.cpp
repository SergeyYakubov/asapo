#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_write.h"
#include "common/networking.h"

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
using ::hidra2::Error;
using ::hidra2::ErrorInterface;
using ::hidra2::FileDescriptor;
using ::hidra2::SocketDescriptor;
using ::hidra2::MockIO;
using hidra2::Request;
using hidra2::RequestHandlerFileWrite;
using ::hidra2::GenericNetworkRequestHeader;

namespace {

class MockRequest: public Request {
  public:
    MockRequest(const GenericNetworkRequestHeader& request_header, SocketDescriptor socket_fd):
        Request(request_header, socket_fd) {};

    MOCK_CONST_METHOD0(GetFileName, std::string());
    MOCK_CONST_METHOD0(GetDataSize, uint64_t());
    MOCK_CONST_METHOD0(GetData, const hidra2::FileData & ());
};

class FileWriteHandlerTests : public Test {
  public:
    RequestHandlerFileWrite handler;
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequest> mock_request;
    void SetUp() override {
        GenericNetworkRequestHeader request_header;
        request_header.data_id = 2;
        mock_request.reset(new MockRequest{request_header, 1});
        handler.io__ = std::unique_ptr<hidra2::IO> {&mock_io};
        /*      ON_CALL(mock_io, Receive_t(socket_fd_, _, data_size_, _)).WillByDefault(
                  DoAll(SetArgPointee<3>(nullptr),
                        Return(0)
                  ));*/
    }
    void TearDown() override {
        handler.io__.release();
    }

};

TEST_F(FileWriteHandlerTests, ErrorWhenZeroFileSize) {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(0))
    ;

    auto err = handler.ProcessRequest(*mock_request);

    ASSERT_THAT(err, Eq(hidra2::ReceiverErrorTemplates::kBadRequest));
}

TEST_F(FileWriteHandlerTests, ErrorWhenTooBigFileSize) {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(hidra2::kMaxFileSize + 1))
    ;

    auto err = handler.ProcessRequest(*mock_request);

    ASSERT_THAT(err, Eq(hidra2::ReceiverErrorTemplates::kBadRequest));
}

TEST_F(FileWriteHandlerTests, CallsWriteFile) {
    std::string expected_file_name = "2.bin";
    uint64_t expected_file_size = 10;
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    hidra2::FileData data;
    EXPECT_CALL(*mock_request, GetData())
    .WillOnce(ReturnRef(data))
    ;

    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;


    EXPECT_CALL(mock_io, WriteDataToFile_t("files/"+expected_file_name, _, expected_file_size))
    .WillOnce(
        //Return(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release())
        Return(nullptr)
    );

    auto err = handler.ProcessRequest(*mock_request);

    ASSERT_THAT(err, Eq(nullptr));
}


}