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
    MockRequest(const std::unique_ptr<GenericNetworkRequestHeader>& request_header, SocketDescriptor socket_fd):
        Request(request_header, socket_fd) {};
    Error Handle() override {
        return Error{Handle_t()};
    };
    MOCK_CONST_METHOD0(Handle_t, ErrorInterface * ());
};

class MockRequestFactory: public hidra2::RequestFactory {
  public:
    std::unique_ptr<Request> GenerateRequest(const std::unique_ptr<GenericNetworkRequestHeader>& request_header,
                                             SocketDescriptor socket_fd,
                                             Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto res = GenerateRequest_t(request_header, socket_fd, &error);
        err->reset(error);
        return std::unique_ptr<Request> {res};
    }

    MOCK_CONST_METHOD3(GenerateRequest_t, Request * (const std::unique_ptr<GenericNetworkRequestHeader>&,
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

TEST_F(ConnectionTests, CallsHandleRequest) {
    std::unique_ptr<GenericNetworkRequestHeader> header{new GenericNetworkRequestHeader};
    auto request = new MockRequest{header, 1};

    EXPECT_CALL(mock_io, ReceiveWithTimeout_t(_, _, _, _, _));

    EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _)).WillOnce(
        Return(request)
    );

    EXPECT_CALL(*request, Handle_t()).WillOnce(
        Return(new hidra2::SimpleError{""})
    );

    connection.Listen();
}

/*
TEST(Constructor, CheckGetAddress) {
    std::string expected_address = "somehost:1234";
    hidra2::Connection networkProducerPeer(0, expected_address);
    ASSERT_THAT(networkProducerPeer.GetAddress(), Eq(expected_address));
}

TEST(CheckIfValidFileSize, ZeroFileSize) {
    hidra2::Connection networkProducerPeer(0, "");
    EXPECT_FALSE(networkProducerPeer.CheckIfValidFileSize(0));
}

TEST(CheckIfValidFileSize, SmallFileSize) {
    hidra2::Connection networkProducerPeer(0, "");
    EXPECT_TRUE(networkProducerPeer.CheckIfValidFileSize(1));
}

TEST(CheckIfValidFileSize, OneGiByteSize) {
    hidra2::Connection networkProducerPeer(0, "");
    EXPECT_TRUE(networkProducerPeer.CheckIfValidFileSize(1024 * 1024 * 1024 * 1));
}

TEST(CheckIfValidFileSize, TwoGiByteSize) {
    hidra2::Connection networkProducerPeer(0, "");
    EXPECT_TRUE(networkProducerPeer.CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2));
}

TEST(CheckIfValidFileSize, MoreThenTwoGiByteSize) {
    hidra2::Connection networkProducerPeer(0, "");
    EXPECT_FALSE(networkProducerPeer.CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2 + 1));
}

TEST(CreateAndOpenFileByFileId, FolderUnknownIOError) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    hidra2::MockIO mockIO;
    NetworkProducerPeerImpl.SetIO__(&mockIO);

    uint64_t expected_file_id = 5435452453867;

    hidra2::Error err;
    EXPECT_CALL(mockIO, CreateNewDirectory_t("files", _))
    .Times(1)
    .WillOnce(
        testing::SetArgPointee<1>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release())
    );
    NetworkProducerPeerImpl.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST(CreateAndOpenFileByFileId, FolderAlreadyExsistsButFileAlreadyExists) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    hidra2::MockIO mockIO;
    NetworkProducerPeerImpl.SetIO__(&mockIO);

    uint64_t expected_file_id = 543545;
    uint64_t expected_fd = 23432;

    hidra2::Error err;
    EXPECT_CALL(mockIO, CreateNewDirectory_t("files", _))
    .Times(1)
    .WillOnce(
        testing::SetArgPointee<1>(hidra2::IOErrorTemplates::kFileAlreadyExists.Generate().release())
    );

    EXPECT_CALL(mockIO, Open_t(
                    "files/" + std::to_string(expected_file_id) + ".bin",
                    hidra2::FileOpenMode::IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS |
                    hidra2::FileOpenMode::IO_OPEN_MODE_RW, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<2>(hidra2::IOErrorTemplates::kFileAlreadyExists.Generate().release()),
            testing::Return(expected_fd)
        )
    );

    hidra2::FileDescriptor fd = NetworkProducerPeerImpl.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kFileAlreadyExists));
    ASSERT_THAT(fd, Eq(expected_fd));
}

TEST(CreateAndOpenFileByFileId, FolderCreatedButFileAlreadyExists) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    hidra2::MockIO mockIO;
    NetworkProducerPeerImpl.SetIO__(&mockIO);

    uint64_t expected_file_id = 543545;

    hidra2::Error err;
    EXPECT_CALL(mockIO, CreateNewDirectory_t("files", _))
    .Times(1)
    .WillOnce(
        testing::SetArgPointee<1>(nullptr)
    );

    EXPECT_CALL(mockIO, Open_t(
                    "files/" + std::to_string(expected_file_id) + ".bin",
                    hidra2::FileOpenMode::IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS |
                    hidra2::FileOpenMode::IO_OPEN_MODE_RW, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<2>(hidra2::IOErrorTemplates::kFileAlreadyExists.Generate().release()),
            testing::Return(-1)
        )
    );

    hidra2::FileDescriptor fd = NetworkProducerPeerImpl.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kFileAlreadyExists));
    ASSERT_THAT(fd, Eq(-1));
}

TEST(CreateAndOpenFileByFileId, Ok) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    hidra2::MockIO mockIO;
    NetworkProducerPeerImpl.SetIO__(&mockIO);

    uint64_t expected_file_id = 543545;
    uint64_t expected_fd = 23432;

    hidra2::Error err;
    EXPECT_CALL(mockIO, CreateNewDirectory_t("files", _))
    .Times(1)
    .WillOnce(
        testing::SetArgPointee<1>(nullptr)
    );

    EXPECT_CALL(mockIO, Open_t(
                    "files/" + std::to_string(expected_file_id) + ".bin",
                    hidra2::FileOpenMode::IO_OPEN_MODE_CREATE_AND_FAIL_IF_EXISTS |
                    hidra2::FileOpenMode::IO_OPEN_MODE_RW, _))
    .Times(1)
    .WillOnce(
        DoAll(
            testing::SetArgPointee<2>(nullptr),
            testing::Return(expected_fd)
        )
    );

    hidra2::FileDescriptor fd = NetworkProducerPeerImpl.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(fd, Eq(expected_fd));
}

TEST(CheckIfValidNetworkOpCode, NormalOpcodeSendData) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    EXPECT_TRUE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode(hidra2::Opcode::kNetOpcodeSendData));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcode) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    EXPECT_FALSE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode(hidra2::Opcode::kNetOpcodeCount));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcodeByNumber) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    EXPECT_FALSE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode((hidra2::Opcode) 90));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcodeByNegativNumber) {
    hidra2::Connection NetworkProducerPeerImpl(0, "");
    //Technically -1 is some big positive number since Opcode is an unsigned 8 bit number
    EXPECT_FALSE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode((hidra2::Opcode) - 1));
}


class ReceiveAndSaveFileFixture : public testing::Test {
  public:
    const hidra2::SocketDescriptor expected_socket_descriptor = 20;
    const std::string expected_address = "somehost:13579";
    const uint64_t expected_file_id = 314322;
    const uint64_t expected_file_size = 784387;
    const FileDescriptor expected_fd = 12643;

    Error err;

    hidra2::MockIO mockIO;
    std::unique_ptr<Connection> networkProducerPeer;

    void SetUp() override {
        networkProducerPeer.reset(new Connection(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(ReceiveAndSaveFileFixture, CheckFileSizeError) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidFileSize(expected_file_size))
    .WillOnce(
        Return(false)
    );

    networkProducerPeer->ReceiveAndSaveFile(expected_file_id, expected_file_size, &err);

    ASSERT_THAT(err, Eq(hidra2::ErrorTemplates::kMemoryAllocationError));
}

TEST_F(ReceiveAndSaveFileFixture, CheckErrorWhenFileAlreadyExists) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidFileSize_t(expected_file_size))
    .WillOnce(
        Return(true)
    );

    EXPECT_CALL(*networkProducerPeer, CreateAndOpenFileByFileId_t(expected_file_id, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(hidra2::IOErrorTemplates::kFileAlreadyExists.Generate().release()),
                  Return(expected_fd))
             );

    EXPECT_CALL(mockIO, Skip_t(_, _, _));

    networkProducerPeer->ReceiveAndSaveFile(expected_file_id, expected_file_size, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kFileAlreadyExists));
}

TEST_F(ReceiveAndSaveFileFixture, CheckErrorWhenUnknownErrorWhileOpenFile) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidFileSize_t(expected_file_size))
    .WillOnce(
        Return(true)
    );

    EXPECT_CALL(*networkProducerPeer, CreateAndOpenFileByFileId_t(expected_file_id, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(expected_fd))
             );

    networkProducerPeer->ReceiveAndSaveFile(expected_file_id, expected_file_size, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(ReceiveAndSaveFileFixture, CheckErrorWhenUnknownErrorWhileReceivingData) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidFileSize_t(expected_file_size))
    .WillOnce(
        Return(true)
    );

    EXPECT_CALL(*networkProducerPeer, CreateAndOpenFileByFileId_t(expected_file_id, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(nullptr),
                  Return(expected_fd)
              ));

    EXPECT_CALL(mockIO, Receive_t(expected_socket_descriptor, _, expected_file_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(-1)
              ));

    networkProducerPeer->ReceiveAndSaveFile(expected_file_id, expected_file_size, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(ReceiveAndSaveFileFixture, CheckErrorWhenUnknownErrorWhileSavingData) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidFileSize_t(expected_file_size))
    .WillOnce(
        Return(true)
    );

    EXPECT_CALL(*networkProducerPeer, CreateAndOpenFileByFileId_t(expected_file_id, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(nullptr),
                  Return(expected_fd)
              ));

    EXPECT_CALL(mockIO, Receive_t(expected_socket_descriptor, _, expected_file_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(nullptr),
                  Return(expected_file_size)
              ));

    EXPECT_CALL(mockIO, Write_t(expected_fd, _, expected_file_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(expected_file_size)
              ));

    networkProducerPeer->ReceiveAndSaveFile(expected_file_id, expected_file_size, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(ReceiveAndSaveFileFixture, Ok) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidFileSize_t(expected_file_size))
    .WillOnce(
        Return(true)
    );

    EXPECT_CALL(*networkProducerPeer, CreateAndOpenFileByFileId_t(expected_file_id, _))
    .WillOnce(DoAll(
                  SetArgPointee<1>(nullptr),
                  Return(expected_fd)
              ));

    EXPECT_CALL(mockIO, Receive_t(expected_socket_descriptor, _, expected_file_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(nullptr),
                  Return(expected_file_size)
              ));

    EXPECT_CALL(mockIO, Write_t(expected_fd, _, expected_file_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(nullptr),
                  Return(expected_file_size)
              ));

    networkProducerPeer->ReceiveAndSaveFile(expected_file_id, expected_file_size, &err);

    ASSERT_THAT(err, Eq(nullptr));
}

class HandleSendDataRequestMock : public ReceiveAndSaveFileMock {
  public:
    HandleSendDataRequestMock(SocketDescriptor socket_fd, const std::string& address)
        : ReceiveAndSaveFileMock(socket_fd, address) {}

    void ReceiveAndSaveFile(uint64_t data_id, size_t data_size, Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        ReceiveAndSaveFile_t(data_id, data_size, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(ReceiveAndSaveFile_t, void(uint64_t
                                                  data_id, size_t
                                                  data_size, ErrorInterface * *err));
};

class HandleSendDataRequestFixture : public ReceiveAndSaveFileFixture {
  public:
    std::unique_ptr<HandleSendDataRequestMock> networkProducerPeer;

    GenericNetworkRequestHeader send_data_request{};
    SendDataResponse send_data_response{};

    void SetUp() override {
        send_data_request.data_id = expected_file_id;
        send_data_request.data_size = expected_file_size;

        networkProducerPeer.reset(new HandleSendDataRequestMock(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(HandleSendDataRequestFixture, Ok) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, ReceiveAndSaveFile_t(expected_file_id, expected_file_size, _))
    .WillOnce(
        SetArgPointee<2>(nullptr)
    );

    networkProducerPeer->HandleSendDataRequest(&send_data_request, &send_data_response, &err);

    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::kNetErrorNoError));
    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::kNetErrorNoError));
}

TEST_F(HandleSendDataRequestFixture, CheckErrorCodeWhenFileIdIsAlreadyUsed) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, ReceiveAndSaveFile_t(expected_file_id, expected_file_size, _))
    .WillOnce(
        SetArgPointee<2>(hidra2::IOErrorTemplates::kFileAlreadyExists.Generate().release())
    );

    networkProducerPeer->HandleSendDataRequest(&send_data_request, &send_data_response, &err);

    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::kNetErrorFileIdAlreadyInUse));
}

TEST_F(HandleSendDataRequestFixture, CheckErrorCodeWhenUnexpectedError) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, ReceiveAndSaveFile_t(expected_file_id, expected_file_size, _))
    .WillOnce(
        SetArgPointee<2>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release())
    );

    networkProducerPeer->HandleSendDataRequest(&send_data_request, &send_data_response, &err);

    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::kNetErrorInternalServerError));
}

class HandleGenericRequestMock : public HandleSendDataRequestMock {
  public:
    HandleGenericRequestMock(SocketDescriptor socket_fd, const std::string& address)
        : HandleSendDataRequestMock(socket_fd, address) {}

    void HandleSendDataRequest(const GenericNetworkRequestHeader* request, SendDataResponse* response, Error* err) noexcept override {
        ErrorInterface* error = nullptr;
        HandleSendDataRequest_t(request, response, &error);
        err->reset(error);
    }
    MOCK_METHOD3(HandleSendDataRequest_t, void(const GenericNetworkRequestHeader* request, SendDataResponse* response,
                                               ErrorInterface** err));

    bool CheckIfValidNetworkOpCode(Opcode opcode) const noexcept override {
        return CheckIfValidNetworkOpCode_t(opcode);
    }
    MOCK_CONST_METHOD1(CheckIfValidNetworkOpCode_t, bool(Opcode opcode));

};

class HandleGenericRequestFixture : public HandleSendDataRequestFixture {
  public:
    std::unique_ptr<HandleGenericRequestMock> networkProducerPeer;

    GenericNetworkRequestHeader generic_request{};
    SendDataResponse generic_response{};

    uint64_t expected_request_id = 423423;
    size_t expected_send_data_buffer_size = sizeof(GenericNetworkRequestHeader) - sizeof(GenericNetworkRequestHeader);
    size_t expected_send_data_response_size = sizeof(SendDataResponse);

    void SetUp() override {
        networkProducerPeer.reset(new HandleGenericRequestMock(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(HandleGenericRequestFixture, CheckIfCheckIfValidNetworkOpCodeIsCalled) {
    err = nullptr;

    Opcode expected_opcode = (Opcode) 931;

    generic_request.op_code = expected_opcode;
    generic_request.data_size = expected_file_size;
    generic_request.data_id = expected_file_id;
    generic_request.request_id = expected_request_id;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidNetworkOpCode_t(expected_opcode))
    .WillOnce(
        Return(false)
    );

    size_t return_size = networkProducerPeer->HandleGenericRequest(&generic_request, &generic_response, &err);

    ASSERT_THAT(return_size, Eq(0));
    ASSERT_THAT(err, Eq(hidra2::ReceiverErrorTemplates::kInvalidOpCode));
}

TEST_F(HandleGenericRequestFixture, CheckErrorWhenReceiveFails) {

    err = nullptr;

    Opcode expected_opcode = Opcode::kNetOpcodeSendData;

    generic_request.op_code = expected_opcode;
    generic_request.data_size = expected_file_size;
    generic_request.data_id = expected_file_id;
    generic_request.request_id = expected_request_id;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidNetworkOpCode_t(expected_opcode))
    .WillOnce(
        Return(true)
    );
    EXPECT_CALL(mockIO, Receive_t(expected_socket_descriptor, _, expected_send_data_buffer_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(sizeof(expected_send_data_buffer_size))
              ));
    size_t return_size = networkProducerPeer->HandleGenericRequest(&generic_request, &generic_response, &err);

    ASSERT_THAT(return_size, Eq(0));
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(HandleGenericRequestFixture, CheckErrorWhenHandlerFails) {

    err = nullptr;

    Opcode expected_opcode = Opcode::kNetOpcodeSendData;

    generic_request.op_code = expected_opcode;
    generic_request.data_size = expected_file_size;
    generic_request.data_id = expected_file_id;
    generic_request.request_id = expected_request_id;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidNetworkOpCode_t(expected_opcode))
    .WillOnce(
        Return(true)
    );

    EXPECT_CALL(mockIO, Receive_t(expected_socket_descriptor, _, expected_send_data_buffer_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(nullptr),
                  Return(sizeof(expected_send_data_buffer_size))
              ));

    EXPECT_CALL(*networkProducerPeer, HandleSendDataRequest_t(_, _, _))
    .WillOnce(
        SetArgPointee<2>(hidra2::ErrorTemplates::kMemoryAllocationError.Generate().release())
    );

    size_t return_size = networkProducerPeer->HandleGenericRequest(&generic_request, &generic_response, &err);

    ASSERT_THAT(return_size, Eq(0));
    ASSERT_THAT(err, Eq(hidra2::ErrorTemplates::kMemoryAllocationError));
}

TEST_F(HandleGenericRequestFixture, Ok) {

    err = nullptr;

    Opcode expected_opcode = Opcode::kNetOpcodeSendData;

    generic_request.op_code = expected_opcode;
    generic_request.data_size = expected_file_size;
    generic_request.data_id = expected_file_id;
    generic_request.request_id = expected_request_id;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidNetworkOpCode_t(expected_opcode))
    .WillOnce(
        Return(true)
    );

    EXPECT_CALL(mockIO, Receive_t(expected_socket_descriptor, _, expected_send_data_buffer_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(nullptr),
                  Return(sizeof(expected_send_data_buffer_size))
              ));

    EXPECT_CALL(*networkProducerPeer, HandleSendDataRequest_t(_, _, _))
    .WillOnce(
        SetArgPointee<2>(nullptr)
    );

    size_t return_size = networkProducerPeer->HandleGenericRequest(&generic_request, &generic_response, &err);

    ASSERT_THAT(return_size, Eq(expected_send_data_response_size));
    ASSERT_THAT(err, Eq(nullptr));
}

class HandleRawRequestBufferMock : public HandleGenericRequestMock {
  public:
    HandleRawRequestBufferMock(SocketDescriptor socket_fd, const std::string& address)
        : HandleGenericRequestMock(socket_fd, address) {}

    size_t HandleGenericRequest(GenericNetworkRequestHeader* request, GenericNetworkResponse* response,
                                Error* err) noexcept override {
        ErrorInterface* error = nullptr;
        auto data = HandleGenericRequest_t(request, response, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD3(HandleGenericRequest_t, size_t(GenericNetworkRequestHeader* request, GenericNetworkResponse* response,
                                                ErrorInterface** err));
};

class HandleRawRequestBufferFixture : public HandleGenericRequestFixture {
  public:
    std::unique_ptr<HandleRawRequestBufferMock> networkProducerPeer;

    void SetUp() override {
        networkProducerPeer.reset(new HandleRawRequestBufferMock(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(HandleRawRequestBufferFixture, CheckErrorWhenHandleGenericRequestFails) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, HandleGenericRequest_t(_, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0)
              ));

    networkProducerPeer->HandleRawRequestBuffer(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(HandleRawRequestBufferFixture, ZeroReturnSize) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, HandleGenericRequest_t(_, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(nullptr),
                  Return(0)
              ));

    networkProducerPeer->HandleRawRequestBuffer(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(HandleRawRequestBufferFixture, SendResponseFaild) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, HandleGenericRequest_t(_, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(nullptr),
                  Return(expected_send_data_response_size)
              ));


    EXPECT_CALL(mockIO, Send_t(expected_socket_descriptor, _, expected_send_data_response_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0)
              ));

    networkProducerPeer->HandleRawRequestBuffer(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(HandleRawRequestBufferFixture, Ok) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, HandleGenericRequest_t(_, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<2>(nullptr),
                  Return(expected_send_data_response_size)
              ));


    EXPECT_CALL(mockIO, Send_t(expected_socket_descriptor, _, expected_send_data_response_size, _))
    .WillOnce(DoAll(
                  SetArgPointee<3>(nullptr),
                  Return(expected_send_data_response_size)
              ));

    networkProducerPeer->HandleRawRequestBuffer(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(nullptr));
}

class InternalPeerReceiverDoWorkMock : public HandleRawRequestBufferMock {
  public:
    InternalPeerReceiverDoWorkMock(SocketDescriptor socket_fd, const std::string& address)
        : HandleRawRequestBufferMock(socket_fd, address) {}

    void HandleRawRequestBuffer(GenericNetworkRequestHeader* request, GenericNetworkResponse* response,
                                Error* err) noexcept override {
        ErrorInterface* error = nullptr;
        HandleRawRequestBuffer_t(request, response, &error);
        err->reset(error);
    }
    MOCK_METHOD3(HandleRawRequestBuffer_t, void(GenericNetworkRequestHeader* request, GenericNetworkResponse* response,
                                                ErrorInterface** err));
};

class InternalPeerReceiverDoWorkFixture : public HandleRawRequestBufferFixture {
  public:
    std::unique_ptr<InternalPeerReceiverDoWorkMock> networkProducerPeer;

    size_t expected_generic_request_size = sizeof(GenericNetworkRequestHeader);

    void SetUp() override {
        networkProducerPeer.reset(new InternalPeerReceiverDoWorkMock(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(InternalPeerReceiverDoWorkFixture, CheckErrorWhenReceiveWithTimeoutFails) {
    err = nullptr;

    EXPECT_CALL(mockIO, ReceiveWithTimeout_t(expected_socket_descriptor, _, expected_generic_request_size, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<4>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release()),
                  Return(0)
              ));

    networkProducerPeer->InternalPeerReceiverDoWork(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(InternalPeerReceiverDoWorkFixture, CheckErrorWhenReceiveWithTimeoutTimeouts) {
    err = nullptr;

    EXPECT_CALL(mockIO, ReceiveWithTimeout_t(expected_socket_descriptor, _, expected_generic_request_size, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<4>(hidra2::IOErrorTemplates::kTimeout.Generate().release()),
                  Return(0)
              ));

    networkProducerPeer->InternalPeerReceiverDoWork(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(InternalPeerReceiverDoWorkFixture, HandleRawRequestBufferFails) {
    err = nullptr;

    EXPECT_CALL(mockIO, ReceiveWithTimeout_t(expected_socket_descriptor, _, expected_generic_request_size, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<4>(nullptr),
                  Return(0)
              ));

    EXPECT_CALL(*networkProducerPeer, HandleRawRequestBuffer_t(_, _, _))
    .WillOnce(
        SetArgPointee<2>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release())
    );

    networkProducerPeer->InternalPeerReceiverDoWork(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(InternalPeerReceiverDoWorkFixture, Ok) {
    err = nullptr;

    EXPECT_CALL(mockIO, ReceiveWithTimeout_t(expected_socket_descriptor, _, expected_generic_request_size, _, _))
    .WillOnce(DoAll(
                  SetArgPointee<4>(nullptr),
                  Return(0)
              ));

    EXPECT_CALL(*networkProducerPeer, HandleRawRequestBuffer_t(_, _, _))
    .WillOnce(
        SetArgPointee<2>(nullptr)
    );

    networkProducerPeer->InternalPeerReceiverDoWork(&generic_request, &generic_response, &err);

    ASSERT_THAT(err, Eq(nullptr));
}

class StartPeerListenerMock : public InternalPeerReceiverDoWorkMock {
  public:
    StartPeerListenerMock(SocketDescriptor socket_fd, const std::string& address)
        : InternalPeerReceiverDoWorkMock(socket_fd, address) {}
};

class StartPeerListenerFixture : public InternalPeerReceiverDoWorkFixture {
  public:
    std::unique_ptr<StartPeerListenerMock> networkProducerPeer;

    void SetUp() override {
        networkProducerPeer.reset(new StartPeerListenerMock(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(StartPeerListenerFixture, Ok) {

    EXPECT_CALL(mockIO, NewThread_t(_));

    networkProducerPeer->Listen();
}

TEST_F(StartPeerListenerFixture, AlreadyListening) {

    EXPECT_CALL(mockIO, NewThread_t(_)).Times(1);

    networkProducerPeer->Listen();
}
*/
}
