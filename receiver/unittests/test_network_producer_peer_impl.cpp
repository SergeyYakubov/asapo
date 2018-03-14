#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/network_producer_peer_impl.h"

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Mock;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::hidra2::Error;
using ::hidra2::ErrorInterface;
using ::hidra2::FileDescriptor;
using ::hidra2::SocketDescriptor;
using ::hidra2::SendDataRequest;
using ::hidra2::SendDataResponse;
using ::hidra2::GenericNetworkRequest;
using ::hidra2::GenericNetworkResponse;
using ::hidra2::Opcode;

namespace {

TEST(Constructor, CheckGetAddress) {
    std::string expected_address = "somehost:1234";
    hidra2::NetworkProducerPeerImpl networkProducerPeer(0, expected_address);
    ASSERT_THAT(networkProducerPeer.GetAddress(), Eq(expected_address));
}

TEST(CheckIfValidFileSize, ZeroFileSize) {
    hidra2::NetworkProducerPeerImpl networkProducerPeer(0, "");
    EXPECT_FALSE(networkProducerPeer.CheckIfValidFileSize(0));
}

TEST(CheckIfValidFileSize, SmallFileSize) {
    hidra2::NetworkProducerPeerImpl networkProducerPeer(0, "");
    EXPECT_TRUE(networkProducerPeer.CheckIfValidFileSize(1));
}

TEST(CheckIfValidFileSize, OneGiByteSize) {
    hidra2::NetworkProducerPeerImpl networkProducerPeer(0, "");
    EXPECT_TRUE(networkProducerPeer.CheckIfValidFileSize(1024 * 1024 * 1024 * 1));
}

TEST(CheckIfValidFileSize, TwoGiByteSize) {
    hidra2::NetworkProducerPeerImpl networkProducerPeer(0, "");
    EXPECT_TRUE(networkProducerPeer.CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2));
}

TEST(CheckIfValidFileSize, MoreThenTwoGiByteSize) {
    hidra2::NetworkProducerPeerImpl networkProducerPeer(0, "");
    EXPECT_FALSE(networkProducerPeer.CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2 + 1));
}

TEST(CreateAndOpenFileByFileId, FolderUnknownIOError) {
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
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
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
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
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
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
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
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
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
    EXPECT_TRUE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode(hidra2::Opcode::kNetOpcodeSendData));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcode) {
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
    EXPECT_FALSE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode(hidra2::Opcode::kNetOpcodeCount));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcodeByNumber) {
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
    EXPECT_FALSE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode((hidra2::Opcode)90));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcodeByNegativNumber) {
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
    //Technically -1 is some big positive number since Opcode is an unsigned 8 bit number
    EXPECT_FALSE(NetworkProducerPeerImpl.CheckIfValidNetworkOpCode((hidra2::Opcode) - 1));
}

class ReceiveAndSaveFileMock : public hidra2::NetworkProducerPeerImpl {
 public:
    ReceiveAndSaveFileMock(hidra2::SocketDescriptor socket_fd, const std::string &address) : NetworkProducerPeerImpl(
        socket_fd,
        address) {}

    FileDescriptor CreateAndOpenFileByFileId(uint64_t file_id, Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto data = CreateAndOpenFileByFileId_t(file_id, &error);
        err->reset(error);
        return data;
    }
    MOCK_CONST_METHOD2(CreateAndOpenFileByFileId_t, FileDescriptor(uint64_t file_id, ErrorInterface** err));


    bool CheckIfValidFileSize(size_t file_size) const noexcept override {
        return CheckIfValidFileSize_t(file_size);
    }
    MOCK_CONST_METHOD1(CheckIfValidFileSize_t, bool(size_t file_size));

};

class ReceiveAndSaveFileFixture : public testing::Test {
  public:
    const hidra2::SocketDescriptor  expected_socket_descriptor = 20;
    const std::string               expected_address = "somehost:13579";
    const uint64_t                  expected_file_id = 314322;
    const uint64_t                  expected_file_size = 784387;
    const FileDescriptor            expected_fd = 12643;

    Error err;

    hidra2::MockIO mockIO;
    std::unique_ptr<ReceiveAndSaveFileMock> networkProducerPeer;

    void SetUp() override {
        networkProducerPeer.reset(new ReceiveAndSaveFileMock(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(ReceiveAndSaveFileFixture, CheckFileSizeError) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, CheckIfValidFileSize_t(expected_file_size))
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
    HandleSendDataRequestMock(SocketDescriptor socket_fd, const std::string &address)
        : ReceiveAndSaveFileMock(socket_fd, address) {}

    void ReceiveAndSaveFile(uint64_t file_id, size_t file_size, Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        ReceiveAndSaveFile_t(file_id, file_size, &error);
        err->reset(error);
    }
    MOCK_CONST_METHOD3(ReceiveAndSaveFile_t, void(uint64_t file_id, size_t file_size, ErrorInterface** err));
};

class HandleSendDataRequestFixture : public ReceiveAndSaveFileFixture {
 public:
    std::unique_ptr<HandleSendDataRequestMock> networkProducerPeer;

    SendDataRequest send_data_request{};
    SendDataResponse send_data_response{};

    void SetUp() override {
        send_data_request.file_id = expected_file_id;
        send_data_request.file_size = expected_file_size;

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

    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::NET_ERR__NO_ERROR));
    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::NET_ERR__NO_ERROR));
}

TEST_F(HandleSendDataRequestFixture, CheckErrorCodeWhenFileIdIsAlreadyUsed) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, ReceiveAndSaveFile_t(expected_file_id, expected_file_size, _))
        .WillOnce(
            SetArgPointee<2>(hidra2::IOErrorTemplates::kFileAlreadyExists.Generate().release())
        );

    networkProducerPeer->HandleSendDataRequest(&send_data_request, &send_data_response, &err);

    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::NET_ERR__FILEID_ALREADY_IN_USE));
}

TEST_F(HandleSendDataRequestFixture, CheckErrorCodeWhenUnexpectedError) {
    err = nullptr;

    EXPECT_CALL(*networkProducerPeer, ReceiveAndSaveFile_t(expected_file_id, expected_file_size, _))
        .WillOnce(
            SetArgPointee<2>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release())
        );

    networkProducerPeer->HandleSendDataRequest(&send_data_request, &send_data_response, &err);

    ASSERT_THAT(send_data_response.error_code, Eq(hidra2::NET_ERR__INTERNAL_SERVER_ERROR));
}

class HandleGenericRequestMock : public HandleSendDataRequestMock {
 public:
    HandleGenericRequestMock(SocketDescriptor socket_fd, const std::string &address)
        : HandleSendDataRequestMock(socket_fd, address) {}

    size_t HandleGenericRequest(GenericNetworkRequest* request, GenericNetworkResponse* response, Error* err) noexcept override {
        ErrorInterface* error = nullptr;
        auto data = HandleGenericRequest_t(request, response, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD3(HandleGenericRequest_t, size_t(GenericNetworkRequest* request, GenericNetworkResponse* response, ErrorInterface** err));


    bool CheckIfValidNetworkOpCode(Opcode opcode) const noexcept override {
        return HandleGenericRequest_t(opcode);
    }
    MOCK_CONST_METHOD1(HandleGenericRequest_t, bool(Opcode opcode));


};

class HandleGenericRequestFixture : public HandleSendDataRequestFixture {
 public:
    std::unique_ptr<HandleGenericRequestMock> networkProducerPeer;

    SendDataRequest generic_request{};
    SendDataResponse generic_response{};

    void SetUp() override {
        networkProducerPeer.reset(new HandleGenericRequestMock(expected_socket_descriptor, expected_address));
        networkProducerPeer->SetIO__(&mockIO);
    }
};

TEST_F(HandleGenericRequestFixture, DebugTest) {
    err = nullptr;

    ASSERT_TRUE(true);
}


}
