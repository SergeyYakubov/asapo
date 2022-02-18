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

using namespace testing;
using namespace asapo;

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
    std::string info_str =
        R"({"lastId":10,"name":"stream","timestampCreated":1000000,"timestampLast":2000000,"finished":false,"nextStream":""})";
    asapo::StreamInfo expected_stream_info;
    void SetUp() override {
        GenericRequestHeader request_header;
        expected_stream_info.last_id = 10;
        expected_stream_info.name = expected_stream;
        expected_stream_info.timestamp_created = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(
                                                     1));
        expected_stream_info.timestamp_lastentry = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(
                                                       2));
        request_header.data_id = 0;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr, nullptr});
        SetDefaultRequestCalls(mock_request.get(),expected_beamtime_id);
    }
    void TearDown() override {
        handler.db_client__.release();
    }
};

TEST_F(DbMetaStreamInfoTests, CallsUpdate) {
    SetReceiverConfig(config, "none");

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillRepeatedly(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetDataSource()).WillRepeatedly(ReturnRef(expected_data_source));

    EXPECT_CALL(*mock_request, GetStream())
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
                                         HasSubstr(expected_beamtime_id)
                                        )
                                  )
               );

    handler.ProcessRequest(mock_request.get());
}

}
