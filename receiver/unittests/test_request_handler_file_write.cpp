#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_write.h"
#include "common/networking.h"

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
using ::hidra2::MockIO;
using hidra2::Request;
using hidra2::RequestHandlerFileWrite;
using ::hidra2::GenericNetworkRequestHeader;

namespace {

class MockRequest: public Request {
  public:
    MockRequest(const GenericNetworkRequestHeader& request_header, SocketDescriptor socket_fd):
        Request(request_header, socket_fd) {};

    MOCK_CONST_METHOD0(GetFileName, std::string());
    MOCK_CONST_METHOD0(GetDataSize, uint64_t());
    MOCK_CONST_METHOD0(GetData, const hidra2::FileData & ());
};

class FileWriteHandlerTests : public Test {
  public:
    RequestHandlerFileWrite handler;
    NiceMock<MockIO> mock_io;
    void SetUp() override {
        handler.io__ = std::unique_ptr<hidra2::IO> {&mock_io};
        /*      ON_CALL(mock_io, Receive_t(socket_fd_, _, data_size_, _)).WillByDefault(
                  DoAll(SetArgPointee<3>(nullptr),
                        Return(0)
                  ));*/
    }
    void TearDown() override {
        handler.io__.release();
    }

};

}