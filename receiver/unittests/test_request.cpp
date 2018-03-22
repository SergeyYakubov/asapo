#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/send_data_request.h"

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

class FactoryTests : public Test {
  public:
    hidra2::RequestFactory factory;
    Error err{nullptr};
    std::unique_ptr<GenericNetworkRequestHeader> generic_request_buffer{new GenericNetworkRequestHeader};
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST_F(FactoryTests, ErrorOnWrongCode) {
    generic_request_buffer->op_code = hidra2::Opcode::kNetOpcodeUnknownOp;
    auto request = factory.GenerateRequest(generic_request_buffer, 1, &err);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(FactoryTests, ReturnsSendDataRequestOnkNetOpcodeSendDataCode) {
    generic_request_buffer->op_code = hidra2::Opcode::kNetOpcodeSendData;
    auto request = factory.GenerateRequest(generic_request_buffer, 1, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<hidra2::SendDataRequest*>(request.get()), Ne(nullptr));
}


class RequestTests : public Test {
  public:
    std::unique_ptr<GenericNetworkRequestHeader> generic_request_buffer{new GenericNetworkRequestHeader};
    hidra2::SocketDescriptor socket_fd_{1};
    uint64_t data_size_ {100};
    std::unique_ptr<Request> request;
    MockIO mock_io;
    void SetUp() override {
        generic_request_buffer->data_size = data_size_;
        request.reset(new Request{generic_request_buffer, socket_fd_});
        request->io__ = std::unique_ptr<hidra2::IO> {&mock_io};;
//      ON_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _)).
//          WillByDefault(DoAll(testing::SetArgPointee<4>(nullptr),
//                              testing::Return(0)));
//      EXPECT_CALL(mock_io, CloseSocket_t(_, _));

    }
    void TearDown() override {
        request->io__.release();
    }

};

TEST_F(RequestTests, HandleReturnsErrorOnMemoryAllocation) {
    generic_request_buffer->data_size = -1;
    request->io__.release();
    request.reset(new Request{generic_request_buffer, socket_fd_});
    auto err = request->Handle();

    ASSERT_THAT(err, Eq(hidra2::ErrorTemplates::kMemoryAllocationError));

}

TEST_F(RequestTests, HandleDoesNotReceiveEmptyData) {
    generic_request_buffer->data_size = 0;
    request->io__.release();
    request.reset(new Request{generic_request_buffer, socket_fd_});
    request->io__ = std::unique_ptr<hidra2::IO> {&mock_io};;

    EXPECT_CALL(mock_io, Receive_t(_, _, _, _)).Times(0);

    auto err = request->Handle();

    ASSERT_THAT(err, Eq(nullptr));
}



TEST_F(RequestTests, HandleReturnsErrorOnDataReceive) {
    EXPECT_CALL(mock_io, Receive_t(socket_fd_, _, data_size_, _)).WillOnce(
        DoAll(SetArgPointee<3>(new hidra2::IOError("", hidra2::IOErrorType::kReadError)),
              Return(0)
             ));

    auto err = request->Handle();

    //socket_fd_
//    EXPECT_CALL(mock_io, Send_t(size_t(SocketDescriptor socket_fd, const void* buf, size_t length, ErrorInterface** err));
    ASSERT_THAT(err, Eq(hidra2::ReceiverErrorTemplates::kConnectionError));

}

}
