#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

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
using ::testing::AllOf;
using ::testing::HasSubstr;


using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;
using asapo::Request;
using asapo::RequestHandlerFileWrite;
using ::asapo::GenericNetworkRequestHeader;

namespace {

TEST(FileWrite, Constructor) {
    RequestHandlerFileWrite handler;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(handler.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(handler.log__), Ne(nullptr));
}


class MockRequestHandler: public Request {
  public:
    MockRequestHandler(const GenericNetworkRequestHeader& request_header, SocketDescriptor socket_fd):
        Request(request_header, socket_fd) {};

    MOCK_CONST_METHOD0(GetFileName, std::string());
    MOCK_CONST_METHOD0(GetDataSize, uint64_t());
    MOCK_CONST_METHOD0(GetData, const asapo::FileData & ());
};

class FileWriteHandlerTests : public Test {
  public:
    RequestHandlerFileWrite handler;
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequestHandler> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_file_name = "2.bin";
    uint64_t expected_file_size = 10;
    void MockRequestData();
    void SetUp() override {
        GenericNetworkRequestHeader request_header;
        request_header.data_id = 2;
        mock_request.reset(new MockRequestHandler{request_header, 1});
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

    auto err = handler.ProcessRequest(*mock_request);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
}

TEST_F(FileWriteHandlerTests, ErrorWhenTooBigFileSize) {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(asapo::kMaxFileSize + 1))
    ;

    auto err = handler.ProcessRequest(*mock_request);

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

    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;
}

TEST_F(FileWriteHandlerTests, CallsWriteFile) {

    MockRequestData();

    EXPECT_CALL(mock_io, WriteDataToFile_t("files/" + expected_file_name, _, expected_file_size))
    .WillOnce(
        Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
    );

    auto err = handler.ProcessRequest(*mock_request);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}


TEST_F(FileWriteHandlerTests, WritesToLog) {

    MockRequestData();

    EXPECT_CALL(mock_io, WriteDataToFile_t(_, _, _))
    .WillOnce(Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("saved file"),
                                         HasSubstr(expected_file_name),
                                         HasSubstr(std::to_string(expected_file_size))
                                        )
                                  )
               );
    handler.ProcessRequest(*mock_request);
}


}