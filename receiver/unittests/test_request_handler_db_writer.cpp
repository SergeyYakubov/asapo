#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockDatabase.h"
#include "unittests/MockLogger.h"

#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_db_write.h"
#include "common/networking.h"
#include "../../common/cpp/src/database/mongodb_client.h"
#include "mock_receiver_config.h"
#include "common/data_structs.h"


using hidra2::FileInfo;
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


using ::hidra2::Error;
using ::hidra2::ErrorInterface;
using ::hidra2::FileDescriptor;
using ::hidra2::SocketDescriptor;
using ::hidra2::MockIO;
using hidra2::Request;
using hidra2::RequestHandlerDbWrite;
using ::hidra2::GenericNetworkRequestHeader;

using hidra2::MockDatabase;
using hidra2::RequestFactory;
using hidra2::SetReceiverConfig;
using hidra2::ReceiverConfig;


namespace {

class MockRequest: public Request {
  public:
    MockRequest(const GenericNetworkRequestHeader& request_header, SocketDescriptor socket_fd):
        Request(request_header, socket_fd) {};

    MOCK_CONST_METHOD0(GetFileName, std::string());
    MOCK_CONST_METHOD0(GetDataSize, uint64_t());
    MOCK_CONST_METHOD0(GetDataID, uint64_t());
    MOCK_CONST_METHOD0(GetData, const hidra2::FileData & ());
};

class DbWriterHandlerTests : public Test {
  public:
    RequestHandlerDbWrite handler;
    NiceMock<MockIO> mock_io;
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<hidra2::MockLogger> mock_logger;
    ReceiverConfig config;
    void SetUp() override {
        GenericNetworkRequestHeader request_header;
        request_header.data_id = 2;
        handler.db_client__ = std::unique_ptr<hidra2::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1});
    }
    void TearDown() override {
        handler.db_client__.release();
    }
};

TEST(DBWritewr, Constructor) {
    RequestHandlerDbWrite handler;
    ASSERT_THAT(dynamic_cast<hidra2::MongoDBClient*>(handler.db_client__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const hidra2::AbstractLogger*>(handler.log__), Ne(nullptr));

}


TEST_F(DbWriterHandlerTests, CheckStatisticEntity) {
    auto entity = handler.GetStatisticEntity();
    ASSERT_THAT(entity, Eq(hidra2::StatisticEntity::kDatabase));
}


TEST_F(DbWriterHandlerTests, ProcessRequestCallsConnectDbWhenNotConnected) {
    config.broker_db_name = "test";
    config.broker_db_uri = "127.0.0.1:27017";
    SetReceiverConfig(config);

    EXPECT_CALL(mock_db, Connect_t("127.0.0.1:27017", "test", hidra2::kDBCollectionName)).
    WillOnce(testing::Return(nullptr));

    auto err = handler.ProcessRequest(*mock_request);
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(DbWriterHandlerTests, ProcessRequestReturnsErrorWhenCannotConnect) {

    EXPECT_CALL(mock_db, Connect_t(_, _, hidra2::kDBCollectionName)).
    WillOnce(testing::Return(new hidra2::SimpleError("")));

    auto err = handler.ProcessRequest(*mock_request);

    ASSERT_THAT(err, Ne(nullptr));

}


TEST_F(DbWriterHandlerTests, ProcessRequestDoesNotCallConnectSecondTime) {

    EXPECT_CALL(mock_db, Connect_t(_, _, hidra2::kDBCollectionName)).
    WillOnce(testing::Return(nullptr));

    handler.ProcessRequest(*mock_request);
    handler.ProcessRequest(*mock_request);
}

MATCHER_P(CompareFileInfo, file, "") {
    if (arg.size != file.size) return false;
    if (arg.name != file.name) return false;
    if (arg.id != file.id) return false;

    return true;
}



TEST_F(DbWriterHandlerTests, CallsInsert) {
    config.broker_db_name = "test";
    config.broker_db_uri = "127.0.0.1:27017";
    SetReceiverConfig(config);

    EXPECT_CALL(mock_db, Connect_t(config.broker_db_uri, config.broker_db_name, hidra2::kDBCollectionName)).
    WillOnce(testing::Return(nullptr));

    std::string expected_file_name = "2.bin";
    uint64_t expected_file_size = 10;
    uint64_t expected_id = 15;
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_file_size))
    ;

    EXPECT_CALL(*mock_request, GetFileName())
    .WillOnce(Return(expected_file_name))
    ;

    EXPECT_CALL(*mock_request, GetDataID())
    .WillOnce(Return(expected_id))
    ;

    FileInfo file_info;
    file_info.size = expected_file_size;
    file_info.name = expected_file_name;
    file_info.id = expected_id;


    EXPECT_CALL(mock_db, Insert_t(CompareFileInfo(file_info), _)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("insert record"),
                                         HasSubstr(config.broker_db_uri),
                                         HasSubstr(config.broker_db_name),
                                         HasSubstr(hidra2::kDBCollectionName)
                                        )
                                  )
               );

    handler.ProcessRequest(*mock_request);
}

}