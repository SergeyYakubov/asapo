#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

#include "../src/statistics_sender_influx_db.h"
#include "../src/statistics_sender.h"
#include "../../common/cpp/src/http_client/curl_http_client.h"
#include "unittests/MockHttpClient.h"
#include "../src/receiver_statistics.h"

#include "../src/receiver_config.h"
#include "mock_receiver_config.h"


using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::HasSubstr;
using ::testing::AllOf;
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::SetArgPointee;

using asapo::StatisticsSenderInfluxDb;
using asapo::MockHttpClient;
using asapo::StatisticsToSend;
using asapo::ReceiverConfig;
using asapo::SetReceiverConfig;

namespace {

TEST(SenderInfluxDb, Constructor) {
    StatisticsSenderInfluxDb sender;
    ASSERT_THAT(dynamic_cast<asapo::CurlHttpClient*>(sender.httpclient__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(sender.log__), Ne(nullptr));
}


class SenderInfluxDbTests : public Test {
  public:
    StatisticsSenderInfluxDb sender;
    MockHttpClient mock_http_client;
    NiceMock<asapo::MockLogger> mock_logger;
    StatisticsToSend statistics;
    ReceiverConfig config;

    void SetUp() override {
        statistics.n_requests = 4;

        statistics.extra_entities.push_back(asapo::ExtraEntity{asapo::kStatisticEntityNames[asapo::StatisticEntity::kDatabase], 0.1});
        statistics.extra_entities.push_back(asapo::ExtraEntity{asapo::kStatisticEntityNames[asapo::StatisticEntity::kNetwork], 0.3});
        statistics.extra_entities.push_back(asapo::ExtraEntity{asapo::kStatisticEntityNames[asapo::StatisticEntity::kDisk], 0.6});

        statistics.elapsed_ms = 100;
        statistics.data_volume = 1000;
        statistics.tags.push_back(std::make_pair("name1", "value1"));
        statistics.tags.push_back(std::make_pair("name2", "value2"));

        config.performance_db_uri = "test_uri";
        config.performance_db_name = "test_name";
        SetReceiverConfig(config, "none");

        sender.httpclient__.reset(&mock_http_client);
        sender.log__ = &mock_logger;
    }
    void TearDown() override {
        sender.httpclient__.release();
    }
};


TEST_F(SenderInfluxDbTests, SendStatisticsCallsPost) {
    std::string expect_string = "statistics,name1=value1,name2=value2 elapsed_ms=100,data_volume=1000,"
                                "n_requests=4,db_share=0.1000,network_share=0.3000,disk_share=0.6000";
    EXPECT_CALL(mock_http_client, Post_t("test_uri/write?db=test_name", expect_string, _, _)).
    WillOnce(
        DoAll(SetArgPointee<3>(new asapo::IOError("Test Read Error", asapo::IOErrorType::kReadError)),
              Return("")
             ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("sending statistics"), HasSubstr(config.performance_db_uri))));


    sender.SendStatistics(statistics);
}

TEST_F(SenderInfluxDbTests, LogErrorWithWrongResponceSendStatistics) {
    EXPECT_CALL(mock_http_client, Post_t(_, _, _, _)).
    WillOnce(
        DoAll(SetArgPointee<2>(asapo::HttpCode::BadRequest), SetArgPointee<3>(nullptr), Return("error response")
             ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("sending statistics"), HasSubstr("error response"))));


    sender.SendStatistics(statistics);
}

TEST_F(SenderInfluxDbTests, LogDebugSendStatistics) {
    EXPECT_CALL(mock_http_client, Post_t(_, _, _, _)).
    WillOnce(
        DoAll(SetArgPointee<3>(nullptr), SetArgPointee<2>(asapo::HttpCode::OK), Return("ok response")
             ));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("sending statistics"),
                                         HasSubstr(config.performance_db_uri),
                                         HasSubstr(config.performance_db_name)
                                        )
                                  )
               );


    sender.SendStatistics(statistics);
}


}
