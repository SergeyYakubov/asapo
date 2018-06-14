#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/statistics.h"
#include "mock_statistics.h"
#include "../src/connection_authorizer.h"
#include "../src/receiver_config.h"
#include "../src/receiver_config_factory.h"
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
using asapo::Connection;
using asapo::MockIO;
using asapo::MockLogger;
using asapo::Request;
using asapo::Statistics;
using asapo::StatisticEntity;
using asapo::MockStatistics;

using asapo::ReceiverConfig;
using asapo::SetReceiverConfig;

namespace {

TEST(Connection, Constructor) {
    Connection connection{0, "some_address", "some_tag"};
    ASSERT_THAT(dynamic_cast<asapo::Statistics*>(connection.statistics__.get()), Ne(nullptr));

    ASSERT_THAT(dynamic_cast<asapo::IO*>(connection.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestFactory*>(connection.request_factory__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(connection.log__), Ne(nullptr));
}

class MockRequestHandler: public Request {
  public:
    MockRequestHandler(const GenericRequestHeader& request_header, SocketDescriptor socket_fd):
        Request(request_header, socket_fd) {};
    Error Handle(std::unique_ptr<Statistics>* statistics) override {
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


class ConnectionTests : public Test {
  public:
    std::string connected_uri{"some_address"};
    NiceMock<MockIO> mock_io;
    MockRequestFactory mock_factory;
    NiceMock<MockStatistics> mock_statictics;
    NiceMock<asapo::MockLogger> mock_logger;
    std::unique_ptr<Connection> connection;
    NiceMock<MockAuthorizer> mock_authorizer;
    Sequence seq_send;
    void MockAuthorize();

    asapo::ReceiverConfig test_config;

  void SetUp() override {
        test_config.authorization_interval = 0;
        SetReceiverConfig(test_config);
        connection = std::unique_ptr<Connection> {new Connection{0, connected_uri, "some_tag"}};
        connection->io__ = std::unique_ptr<asapo::IO> {&mock_io};
        connection->statistics__ = std::unique_ptr<asapo::Statistics> {&mock_statictics};
        connection->request_factory__ = std::unique_ptr<asapo::RequestFactory> {&mock_factory};
        connection->log__ = &mock_logger;
        connection->authorizer__ = std::unique_ptr<asapo::ConnectionAuthorizer> {&mock_authorizer};
        ON_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _)).
        WillByDefault(DoAll(testing::SetArgPointee<4>(nullptr),
                            testing::Return(0)));
        EXPECT_CALL(mock_io, CloseSocket_t(_, _));
        EXPECT_CALL(mock_statictics, Send_t());
        ON_CALL(mock_authorizer, Authorize_t(_,_)).WillByDefault(Return(nullptr));

    }
    void TearDown() override {
        connection->io__.release();
        connection->request_factory__.release();
        connection->authorizer__.release();
        connection->statistics__.release();
    }

};

ACTION_P(A_WriteAuth, op_code) {
    ((asapo::GenericRequestHeader*)arg1)->op_code = op_code;
    strcpy(((asapo::GenericRequestHeader*)arg1)->message, "test");
}


void ConnectionTests::MockAuthorize() {
    EXPECT_CALL(mock_io, Receive_t(_, _, _, _)).WillOnce(
        DoAll(SetArgPointee<3>(nullptr),
              A_WriteAuth(asapo::kOpcodeAuthorize),
              Return(0)
             ));


    EXPECT_CALL(mock_authorizer, Authorize_t(_,_)).Times(testing::AtLeast(1)).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(mock_io, Send_t(_, _, _, _))
    .InSequence(seq_send)
    .WillOnce(
        DoAll(SetArgPointee<3>(nullptr),
              Return(0)
             ));
}



TEST_F(ConnectionTests, ErrorWaitForNewRequest) {

    MockAuthorize();
    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _)).Times(2).
    WillOnce(
        DoAll(SetArgPointee<4>(new asapo::IOError("", asapo::IOErrorType::kTimeout)),
              Return(0)))
    .WillOnce(
        DoAll(SetArgPointee<4>(new asapo::IOError("", asapo::IOErrorType::kUnknownIOError)),
              Return(0))
    );

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("waiting for request"), HasSubstr(connected_uri))));


    connection->Listen();
}

ACTION_P(SaveArg1ToGenericNetworkResponse, value) {
    auto resp =  *static_cast<const GenericNetworkResponse*>(arg1);
    value->error_code = resp.error_code;
    strcpy(value->message, resp.message);
}


TEST_F(ConnectionTests, ErrorOnReadAuthorizationHeader) {

    EXPECT_CALL(mock_io, Receive_t(_, _, _, _)).WillOnce(
        DoAll(SetArgPointee<3>(new asapo::IOError("Test Read Error", asapo::IOErrorType::kReadError)),
              Return(0)
             ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("receive authorization"), HasSubstr(connected_uri))));


    GenericNetworkResponse response;
    EXPECT_CALL(mock_io, Send_t(_, _, sizeof(GenericNetworkResponse), _))
    .WillOnce(
        DoAll(SetArgPointee<3>(nullptr),
              SaveArg1ToGenericNetworkResponse(&response),
              Return(0)
             ));


    connection->Listen();

    ASSERT_THAT(response.error_code, Eq(asapo::NetworkErrorCode::kNetAuthorizationError));
    ASSERT_THAT(response.message, HasSubstr("Test Read Error"));
}



TEST_F(ConnectionTests, ErrorOnSendingAuthorizationResponse) {

    EXPECT_CALL(mock_io, Receive_t(_, _, _, _)).WillOnce(
        DoAll(SetArgPointee<3>(nullptr),
              A_WriteAuth(asapo::kOpcodeAuthorize),
              Return(0)
             ));

    EXPECT_CALL(mock_io, Send_t(_, _, sizeof(GenericNetworkResponse), _)).WillOnce(
        DoAll(SetArgPointee<3>(new asapo::IOError("Test Send Error", asapo::IOErrorType::kUnknownIOError)),
              Return(0)
             ));

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("authorization"), HasSubstr("response"), HasSubstr(connected_uri))));

    connection->Listen();

}

TEST_F(ConnectionTests, AuthorizerReturnsOk) {
    EXPECT_CALL(mock_io, Receive_t(_, _, _, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteAuth(asapo::kOpcodeAuthorize),
                testing::ReturnArg<2>()
            ));

    EXPECT_CALL(mock_authorizer, Authorize_t(StrEq("test"),connected_uri)).
        WillOnce(
        Return(nullptr));

    EXPECT_CALL(mock_io, Send_t(_, _, _, _)).WillOnce(
        DoAll(SetArgPointee<3>(new asapo::IOError("Test Send Error", asapo::IOErrorType::kUnknownIOError)),
              Return(0)
        ));

    connection->Listen();

}

TEST_F(ConnectionTests, ErrorOnWrongAuthHeaderCode) {
    EXPECT_CALL(mock_io, Receive_t(_, _, _, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteAuth(asapo::kOpcodeTransferData),
                Return(0)
            ));

    EXPECT_CALL(mock_authorizer, Authorize_t(StrEq("test"),connected_uri)).Times(0);

    EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("wrong"),HasSubstr("code"), HasSubstr(connected_uri))));

    connection->Listen();
}

TEST_F(ConnectionTests, AuthorizerReturnsError) {
    EXPECT_CALL(mock_io, Receive_t(_, _, _, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteAuth(asapo::kOpcodeAuthorize),
                testing::ReturnArg<2>()
            ));

    EXPECT_CALL(mock_authorizer, Authorize_t(StrEq("test"),connected_uri)).
        WillOnce(
        Return(new asapo::SimpleError{"auth error"}));

    connection->Listen();
}


TEST_F(ConnectionTests, CallsHandleRequest) {

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

    connection->Listen();
}

TEST_F(ConnectionTests, SendsErrorToProducer) {

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


    GenericNetworkResponse response;
    EXPECT_CALL(mock_io, Send_t(_, _, sizeof(GenericNetworkResponse), _))
    .InSequence(seq_send)
    .WillOnce(
        DoAll(SetArgPointee<3>(new asapo::IOError("Test Send Error", asapo::IOErrorType::kUnknownIOError)),
              SaveArg1ToGenericNetworkResponse(&response),
              Return(0)
             ));

    connection->Listen();

    ASSERT_THAT(response.error_code, Eq(asapo::NetworkErrorCode::kNetErrorInternalServerError));

}

void MockExitCycle(const MockIO& mock_io) {

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _))
        .Times(testing::AtLeast(1)).WillRepeatedly(
            DoAll(SetArgPointee<4>(new asapo::IOError("", asapo::IOErrorType::kUnknownIOError)),
                  Return(0))
        );
}


void MockExitCycle(const MockIO& mock_io, MockStatistics& mock_statictics) {
    EXPECT_CALL(mock_statictics, StartTimer_t(StatisticEntity::kNetwork));

    MockExitCycle(mock_io);
}

MockRequestHandler* MockWaitRequest(const MockRequestFactory& mock_factory) {
    GenericRequestHeader header;
    header.data_size = 1;
    auto request = new MockRequestHandler{header, 1};
    EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _)).WillOnce(
        Return(request)
    );
    return request;
}

TEST_F(ConnectionTests, FillsStatistics) {
    InSequence sequence;

    MockAuthorize();

    EXPECT_CALL(mock_statictics, StartTimer_t(StatisticEntity::kNetwork));

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _));

    EXPECT_CALL(mock_statictics, StopTimer_t());

    auto request = MockWaitRequest(mock_factory);

    EXPECT_CALL(*request, Handle_t()).WillOnce(
        Return(nullptr)
    );

    EXPECT_CALL(mock_io, Send_t(_, _, _, _))
    .InSequence(seq_send)
    .WillOnce(
        DoAll(SetArgPointee<3>(nullptr),
              Return(0)
             ));

    EXPECT_CALL(mock_statictics, IncreaseRequestCounter_t());

    EXPECT_CALL(mock_statictics, IncreaseRequestDataVolume_t(1 + sizeof(asapo::GenericRequestHeader) +
                sizeof(asapo::GenericNetworkResponse)));


    EXPECT_CALL(mock_statictics, SendIfNeeded_t());

    EXPECT_CALL(mock_authorizer, Authorize_t(_,_)).WillOnce(Return(nullptr));

    MockExitCycle(mock_io, mock_statictics);

    connection->Listen();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

}


TEST_F(ConnectionTests, AuthorizedHeaderExtractedOnlyOnce) {
    InSequence sequence;

    MockAuthorize();

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _));
    auto request = MockWaitRequest(mock_factory);

    EXPECT_CALL(*request, Handle_t()).WillOnce(
        Return(nullptr)
    );

    EXPECT_CALL(mock_io, Send_t(_, _, _, _))
        .WillOnce(
            DoAll(SetArgPointee<3>(nullptr),
                  Return(0)
            ));

    EXPECT_CALL(mock_authorizer, Authorize_t(_,_)).WillOnce(Return(nullptr));

    EXPECT_CALL(mock_io, Send_t(_, _, _, _)).Times(0);

    MockExitCycle(mock_io);

    connection->Listen();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

}

}
