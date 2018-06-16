#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/statistics.h"
#include "mock_statistics.h"
#include "../src/connection_authorizer.h"
#include "../src/receiver_config.h"
#include "../src/receiver_config_factory.h"
#include "mock_receiver_config.h"

#include "../src/requests_dispatcher.h"


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
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::HasSubstr;
using ::testing::StrEq;
using ::testing::SetArgPointee;
using ::testing::AllOf;
using testing::Sequence;

using asapo::Error;
using asapo::ErrorInterface;
using asapo::FileDescriptor;
using asapo::SocketDescriptor;
using asapo::GenericRequestHeader;
using asapo::SendDataResponse;
using asapo::GenericRequestHeader;
using asapo::GenericNetworkResponse;
using asapo::Opcode;
using asapo::MockIO;
using asapo::MockLogger;
using asapo::Request;
using asapo::Statistics;
using asapo::StatisticEntity;
using asapo::MockStatistics;

using asapo::ReceiverConfig;
using asapo::SetReceiverConfig;

using asapo::SetReceiverConfig;
using asapo::RequestsDispatcher;


namespace {

TEST(RequestDispatcher, Constructor) {
    asapo::Statistics* stat;
    RequestsDispatcher dispatcher{0,  "some_address",stat};
    ASSERT_THAT(dynamic_cast<const asapo::Statistics*>(dispatcher.statistics__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::IO*>(dispatcher.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestFactory*>(dispatcher.request_factory__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(dispatcher.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::ConnectionAuthorizer*>(dispatcher.authorizer__.get()), Ne(nullptr));
}

class MockRequest: public Request {
 public:
  MockRequest(const GenericRequestHeader& request_header, SocketDescriptor socket_fd):
      Request(request_header, socket_fd) {};
  Error Handle(Statistics* statistics) override {
      return Error{Handle_t()};
  };
  MOCK_CONST_METHOD0(Handle_t, ErrorInterface * ());
};


class MockRequestFactory: public asapo::RequestFactory {
 public:
  std::unique_ptr<Request> GenerateRequest(const GenericRequestHeader& request_header,
                                           SocketDescriptor socket_fd,
                                           Error* err) const noexcept override {
      ErrorInterface* error = nullptr;
      auto res = GenerateRequest_t(request_header, socket_fd, &error);
      err->reset(error);
      return std::unique_ptr<Request> {res};
  }

  MOCK_CONST_METHOD3(GenerateRequest_t, Request * (const GenericRequestHeader&,
      SocketDescriptor socket_fd,
      ErrorInterface**));

};

class MockAuthorizer: public asapo::ConnectionAuthorizer {
 public:
  Error Authorize(std::string beamtime_id,std::string uri) const noexcept override {
      return Error{Authorize_t(beamtime_id,uri)};
  }
  MOCK_CONST_METHOD2(Authorize_t, ErrorInterface * (std::string beamtime_id,std::string uri));

};


class RequestsDispatcherTests : public Test {
 public:
  std::unique_ptr<RequestsDispatcher> dispatcher;
  std::string connected_uri{"some_address"};
  NiceMock<MockIO> mock_io;
  MockRequestFactory mock_factory;
  NiceMock<MockStatistics> mock_statictics;
  NiceMock<asapo::MockLogger> mock_logger;
  NiceMock<MockAuthorizer> mock_authorizer;
  Sequence seq_send;
  void MockAuthorize();

  asapo::ReceiverConfig test_config;
  GenericRequestHeader header;
  std::string expected_beamtime_id="beamtime_id";
  MockRequest mock_authorize_request{GenericRequestHeader{asapo::kOpcodeAuthorize,0,0,expected_beamtime_id},1};

  void SetUp() override {
      test_config.authorization_interval = 0;
      SetReceiverConfig(test_config);
      dispatcher = std::unique_ptr<RequestsDispatcher> {new RequestsDispatcher{0, connected_uri, &mock_statictics}};
      dispatcher->io__ = std::unique_ptr<asapo::IO> {&mock_io};
      dispatcher->statistics__ = &mock_statictics;
      dispatcher->request_factory__ = std::unique_ptr<asapo::RequestFactory> {&mock_factory};
      dispatcher->log__ = &mock_logger;
      dispatcher->authorizer__ = std::unique_ptr<asapo::ConnectionAuthorizer> {&mock_authorizer};

  }
  void TearDown() override {
      dispatcher->io__.release();
      dispatcher->request_factory__.release();
      dispatcher->authorizer__.release();
  }
  void MockReceiveRequest(bool error ){
      EXPECT_CALL(mock_io, Receive_t(_, _, _, _))
          .WillOnce(
              DoAll(SetArgPointee<3>(error?asapo::IOErrorTemplates::kUnknownIOError.Generate().release():nullptr),
                    Return(0))
          );

  }
  void MockCreateRequest(bool error ){
      EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _))
          .WillOnce(
              DoAll(SetArgPointee<2>(error?asapo::ReceiverErrorTemplates::kInvalidOpCode.Generate().release():nullptr),
                    Return(nullptr))
          );

  }

};

TEST_F(RequestsDispatcherTests, ErrorReceivetNextRequest) {
    EXPECT_CALL(mock_statictics, StartTimer_t(StatisticEntity::kNetwork));
    MockReceiveRequest(true);
    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("getting next request"), HasSubstr(connected_uri))));

    Error err;
    dispatcher->GetNextRequest(&err);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(RequestsDispatcherTests, ErrorCreatetNextRequest) {
    MockReceiveRequest(false);
    MockCreateRequest(true);
    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("error processing request from"), HasSubstr(connected_uri))));

    Error err;
    dispatcher->GetNextRequest(&err);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInvalidOpCode));
}

TEST_F(RequestsDispatcherTests, OkCreatetNextRequest) {
    MockReceiveRequest(false);
    MockCreateRequest(false);

    Error err;
    dispatcher->GetNextRequest(&err);

    ASSERT_THAT(err, Eq(nullptr));
}


/*

ACTION_P(A_WriteAuth, op_code) {
    ((asapo::GenericRequestHeader*)arg1)->op_code = op_code;
    strcpy(((asapo::GenericRequestHeader*)arg1)->message, "test");
}


ACTION_P(SaveArg1ToGenericNetworkResponse, value) {
    auto resp =  *static_cast<const GenericNetworkResponse*>(arg1);
    value->error_code = resp.error_code;
    strcpy(value->message, resp.message);
}



TEST_F(RequestsDispatcherTests, CallsHandleRequest) {

    GenericRequestHeader header;
    auto request = new MockRequestHandler{header, 1};
    MockAuthorize();

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _));

    EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _)).WillOnce(
        Return(request)
    );

    EXPECT_CALL(*request, Handle_t()).WillOnce(
        Return(new asapo::SimpleError{""})
    );

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("processing request"), HasSubstr(connected_uri))));
    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("processing request"), HasSubstr(connected_uri))));


    EXPECT_CALL(mock_io, Send_t(_, _, _, _))
        .InSequence(seq_send)
        .WillOnce(
            DoAll(SetArgPointee<3>(new asapo::IOError("Test Send Error", asapo::IOErrorType::kUnknownIOError)),
                  Return(0)
            ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("sending response"), HasSubstr(connected_uri))));
    EXPECT_CALL(mock_logger, Info(AllOf(HasSubstr("disconnected"), HasSubstr(connected_uri))));

    dispatcher->Listen();
}



*/

}
