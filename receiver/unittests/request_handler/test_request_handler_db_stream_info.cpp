#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockDatabase.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_db_stream_info.h"
#include "../../../common/cpp/src/database/mongodb_client.h"

#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"
#include "asapo/common/networking.h"
#include "../receiver_mocking.h"

using asapo::MockRequest;
using asapo::FileInfo;
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
using asapo::RequestHandlerDbStreamInfo;
using ::asapo::GenericRequestHeader;

using asapo::MockDatabase;
using asapo::RequestFactory;
using asapo::SetReceiverConfig;
using asapo::ReceiverConfig;


namespace {

class DbMetaStreamInfoTests : public Test {
  public:
    std::string expected_stream = "stream";
    std::string expected_collection_name = std::string(asapo::kDBDataCollectionNamePrefix) + "_" + expected_stream;
    RequestHandlerDbStreamInfo handler{asapo::kDBDataCollectionNamePrefix};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_data_source = "source";
    std::string info_str = R"({"lastId":10,"name":"stream","timestampCreated":1000000,"timestampLast":2000000})";
    asapo::StreamInfo expected_stream_info;
    void SetUp() override {
        GenericRequestHeader request_header;
        expected_stream_info.last_id = 10;
        expected_stream_info.name = expected_stream;
        expected_stream_info.timestamp_created = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1));
        expected_stream_info.timestamp_lastentry = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(2));
        request_header.data_id = 0;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr});
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void TearDown() override {
        handler.db_client__.release();
    }
};

TEST_F(DbMetaStreamInfoTests, CallsUpdate) {
    SetReceiverConfig(config, "none");

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetDataSource()).WillOnce(ReturnRef(expected_data_source));

    EXPECT_CALL(*mock_request, GetStream()).Times(2)
    .WillRepeatedly(Return(expected_stream))
    ;

    EXPECT_CALL(mock_db, Connect_t(config.database_uri, expected_beamtime_id + "_" + expected_data_source)).
    WillOnce(testing::Return(nullptr));


    EXPECT_CALL(mock_db, GetStreamInfo_t(expected_collection_name, _)).
    WillOnce(DoAll(
                 SetArgPointee<1>(expected_stream_info),
                 testing::Return(nullptr)
             ));

    EXPECT_CALL(*mock_request, SetResponseMessage(info_str, asapo::ResponseMessageType::kInfo));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("get stream info"),
                                         HasSubstr(config.database_uri),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(expected_collection_name)
                                        )
                                  )
               );

    handler.ProcessRequest(mock_request.get());
}

}
