#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::hidra2::Error;
using ::hidra2::ErrorInterface;
using ::hidra2::FileDescriptor;
using ::hidra2::SocketDescriptor;
using ::hidra2::GenericNetworkRequestHeader;
using ::hidra2::SendDataResponse;
using ::hidra2::GenericNetworkRequestHeader;
using ::hidra2::GenericNetworkResponse;
using ::hidra2::Opcode;
using ::hidra2::Connection;
using ::hidra2::MockIO;
using hidra2::Request;

namespace {

class MockRequest: public Request {
  public:
    MockRequest(const GenericNetworkRequestHeader& request_header, SocketDescriptor socket_fd):
        Request(request_header, socket_fd) {};
    Error Handle() override {
        return Error{Handle_t()};
    };
    MOCK_CONST_METHOD0(Handle_t, ErrorInterface * ());
};

class MockRequestFactory: public hidra2::RequestFactory {
  public:
    std::unique_ptr<Request> GenerateRequest(const GenericNetworkRequestHeader& request_header,
                                             SocketDescriptor socket_fd,
                                             Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto res = GenerateRequest_t(request_header, socket_fd, &error);
        err->reset(error);
        return std::unique_ptr<Request> {res};
    }

    MOCK_CONST_METHOD3(GenerateRequest_t, Request * (const GenericNetworkRequestHeader&,
                                                     SocketDescriptor socket_fd,
                                                     ErrorInterface**));

};

class ConnectionTests : public Test {
  public:
    Connection connection{0, "some_address"};
    MockIO mock_io;
    MockRequestFactory mock_factory;
    void SetUp() override {
        connection.io__ = std::unique_ptr<hidra2::IO> {&mock_io};;
        connection.request_factory__ = std::unique_ptr<hidra2::RequestFactory> {&mock_factory};
        ON_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _)).
        WillByDefault(DoAll(testing::SetArgPointee<4>(nullptr),
                            testing::Return(0)));
        EXPECT_CALL(mock_io, CloseSocket_t(_, _));

    }
    void TearDown() override {
        connection.io__.release();
        connection.request_factory__.release();
    }

};


TEST_F(ConnectionTests, ErrorWaitForNewRequest) {

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _)).Times(2).
    WillOnce(
        DoAll(SetArgPointee<4>(new hidra2::IOError("", hidra2::IOErrorType::kTimeout)),
              Return(0)))
    .WillOnce(
        DoAll(SetArgPointee<4>(new hidra2::IOError("", hidra2::IOErrorType::kUnknownIOError)),
              Return(0))
    );

    connection.Listen();
}

ACTION_P(SaveArg1ToGenericNetworkResponse, value) {
    auto resp =  *static_cast<const GenericNetworkResponse*>(arg1);
    value->error_code = resp.error_code;
}


TEST_F(ConnectionTests, CallsHandleRequest) {

    GenericNetworkRequestHeader header;
    auto request = new MockRequest{header, 1};

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _));

    EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _)).WillOnce(
        Return(request)
    );

    EXPECT_CALL(*request, Handle_t()).WillOnce(
        Return(new hidra2::SimpleError{""})
    );

    EXPECT_CALL(mock_io, Send_t(_, _, _, _)).WillOnce(
        DoAll(SetArgPointee<3>(new hidra2::IOError("Test Send Error", hidra2::IOErrorType::kUnknownIOError)),
              Return(0)
             ));


    connection.Listen();
}

TEST_F(ConnectionTests, SendsNoErrorToProducer) {

    GenericNetworkRequestHeader header;
    auto request = new MockRequest{header, 1};

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _));

    EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _)).WillOnce(
        Return(request)
    );

    EXPECT_CALL(*request, Handle_t()).WillOnce(
        Return(nullptr)
    );
    GenericNetworkResponse response;
    EXPECT_CALL(mock_io, Send_t(_, _, sizeof(GenericNetworkResponse), _)).WillOnce(
        DoAll(SetArgPointee<3>(new hidra2::IOError("Test Send Error", hidra2::IOErrorType::kUnknownIOError)),
              SaveArg1ToGenericNetworkResponse(&response),
              Return(0)
             ));

    connection.Listen();

    ASSERT_THAT(response.error_code, Eq(hidra2::NetworkErrorCode::kNetErrorNoError));
}

TEST_F(ConnectionTests, SendsErrorToProducer) {

    GenericNetworkRequestHeader header;
    auto request = new MockRequest{header, 1};

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _));

    EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _)).WillOnce(
        Return(request)
    );

    EXPECT_CALL(*request, Handle_t()).WillOnce(
        Return(new hidra2::SimpleError{""})
    );

    GenericNetworkResponse response;
    EXPECT_CALL(mock_io, Send_t(_, _, sizeof(GenericNetworkResponse), _)).WillOnce(
        DoAll(SetArgPointee<3>(new hidra2::IOError("Test Send Error", hidra2::IOErrorType::kUnknownIOError)),
              SaveArg1ToGenericNetworkResponse(&response),
              Return(0)
             ));

    connection.Listen();

    ASSERT_THAT(response.error_code, Eq(hidra2::NetworkErrorCode::kNetErrorInternalServerError));

}

}