#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "unittests/MockLogger.h"
#include "common/error.h"
#include "common/io_error.h"

#include "../src/receiver_discovery_service.h"
#include "unittests/MockHttpClient.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::AllOf;

using ::testing::Test;

using ::testing::InSequence;
using ::testing::HasSubstr;
using testing::SetArgPointee;
using testing::ElementsAre;

using asapo::Error;
using asapo::MockHttpClient;
using asapo::ReceiverDiscoveryService;

std::mutex mutex;

TEST(ReceiversStatus, Constructor) {
    ReceiverDiscoveryService status{"endpoint", 1000};
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(status.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::HttpClient*>(status.httpclient__.get()), Ne(nullptr));
}


class ReceiversStatusTests : public Test {
  public:
    // important to create logger before status, otherwise checks in destructor won't work
    NiceMock<asapo::MockLogger> mock_logger;
    NiceMock<MockHttpClient>* mock_http_client;

    std::string expected_endpoint{"endpoint/asapo-discovery/asapo-receiver"};
    ReceiverDiscoveryService status{"endpoint", 20};

    void SetUp() override {
        mock_http_client = new NiceMock<MockHttpClient>;
        status.httpclient__.reset(mock_http_client);
        status.log__ = &mock_logger;
    }
    void TearDown() override {
    }
};

TEST_F(ReceiversStatusTests, LogWhenHttpError) {
    EXPECT_CALL(*mock_http_client, Get_t(expected_endpoint, _, _))
    .Times(1)
    .WillOnce(
        DoAll(SetArgPointee<2>(new asapo::IOError("Test Read Error", asapo::IOErrorType::kReadError)),
              Return("")
             ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("getting receivers"), HasSubstr(expected_endpoint))));
    status.StartCollectingData();

}

TEST_F(ReceiversStatusTests, LogWhenWhenWrongHttpCode) {
    EXPECT_CALL(*mock_http_client, Get_t(expected_endpoint, _, _))
    .Times(testing::AnyNumber())
    .WillRepeatedly(
        DoAll(SetArgPointee<2>(nullptr),
              SetArgPointee<1>(asapo::HttpCode::BadRequest),
              Return("bad request")
             ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("getting receivers"), HasSubstr(expected_endpoint),
                                         HasSubstr("bad request")))).Times(testing::AtLeast(1));
    status.StartCollectingData();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

}

TEST_F(ReceiversStatusTests, LogWhenWhenCannotReadResponce) {
    EXPECT_CALL(*mock_http_client, Get_t(expected_endpoint, _, _))
    .WillOnce(
        DoAll(SetArgPointee<2>(nullptr),
              SetArgPointee<1>(asapo::HttpCode::OK),
              Return("wrong response")
             ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("getting receivers"), HasSubstr(expected_endpoint),
                                         HasSubstr("parse"))));
    status.StartCollectingData();
}


TEST_F(ReceiversStatusTests, GetsReqestedInformation) {
    std::string json = R"({"Uris":["s1","s2","s3"], "MaxConnections":8})";

    EXPECT_CALL(*mock_http_client, Get_t(expected_endpoint, _, _))
    .Times(testing::AtLeast(1))
    .WillRepeatedly(
        DoAll(SetArgPointee<2>(nullptr),
              SetArgPointee<1>(asapo::HttpCode::OK),
              Return(json)
             ));

    status.StartCollectingData();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    auto nc = status.MaxConnections();
    ASSERT_THAT(nc, Eq(8));
    auto list = status.RotatedUriList(0);
    ASSERT_THAT(list, ElementsAre("s1", "s2", "s3"));

    list = status.RotatedUriList(1);
    ASSERT_THAT(list, ElementsAre("s2", "s3", "s1"));

    list = status.RotatedUriList(2);
    ASSERT_THAT(list, ElementsAre("s3", "s1", "s2"));

    list = status.RotatedUriList(3);
    ASSERT_THAT(list, ElementsAre("s1", "s2", "s3"));

}

TEST_F(ReceiversStatusTests, JoinThreadAtTheEnd) {
    std::string json = R"({"uri_list":["s1","s2","s3"], "max_connections":8})";
    EXPECT_CALL(*mock_http_client, Get_t(expected_endpoint, _, _))
    .Times(testing::AtLeast(1))
    .WillRepeatedly(
        DoAll(SetArgPointee<2>(nullptr),
              SetArgPointee<1>(asapo::HttpCode::OK),
              Return(json)
             ));

    EXPECT_CALL(mock_logger, Debug(HasSubstr("starting receiver discovery")));
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing")));
    status.StartCollectingData();
}

TEST_F(ReceiversStatusTests, InitialMaxConnection) {
    auto nc = status.MaxConnections();
    ASSERT_THAT(nc, Eq(0));
}

TEST_F(ReceiversStatusTests, InitialUriList) {
    auto list = status.RotatedUriList(0);
    ASSERT_THAT(list.size(), Eq(0));
}


}
