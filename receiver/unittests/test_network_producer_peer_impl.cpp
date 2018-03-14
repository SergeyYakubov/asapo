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

namespace {

TEST(Constructor, CheckGetAddress) {
    std::string expected_address = "somehost:1234";
    hidra2::NetworkProducerPeerImpl networkProducerPeer(0, expected_address);
    ASSERT_THAT(networkProducerPeer.GetAddress(), Eq(expected_address));
}

TEST(CheckIfValidFileSize, ZeroFileSize) {
    EXPECT_FALSE(hidra2::NetworkProducerPeerImpl::CheckIfValidFileSize(0));
}

TEST(CheckIfValidFileSize, SmallFileSize) {
    EXPECT_TRUE(hidra2::NetworkProducerPeerImpl::CheckIfValidFileSize(1));
}

TEST(CheckIfValidFileSize, OneGiByteSize) {
    EXPECT_TRUE(hidra2::NetworkProducerPeerImpl::CheckIfValidFileSize(1024 * 1024 * 1024 * 1));
}

TEST(CheckIfValidFileSize, TwoGiByteSize) {
    EXPECT_TRUE(hidra2::NetworkProducerPeerImpl::CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2));
}

TEST(CheckIfValidFileSize, MoreThenTwoGiByteSize) {
    EXPECT_FALSE(hidra2::NetworkProducerPeerImpl::CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2 + 1));
}

TEST(CreateAndOpenFileByFileId, FolderUnknownIOError) {
    hidra2::NetworkProducerPeerImpl NetworkProducerPeerImpl(0, "");
    hidra2::MockIO mockIO;
    NetworkProducerPeerImpl.__set_io(&mockIO);

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
    NetworkProducerPeerImpl.__set_io(&mockIO);

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
    NetworkProducerPeerImpl.__set_io(&mockIO);

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
    NetworkProducerPeerImpl.__set_io(&mockIO);

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
    EXPECT_TRUE(hidra2::NetworkProducerPeerImpl::CheckIfValidNetworkOpCode(hidra2::Opcode::kNetOpcodeSendData));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcode) {
    EXPECT_FALSE(hidra2::NetworkProducerPeerImpl::CheckIfValidNetworkOpCode(hidra2::Opcode::kNetOpcodeCount));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcodeByNumber) {
    EXPECT_FALSE(hidra2::NetworkProducerPeerImpl::CheckIfValidNetworkOpCode((hidra2::Opcode)90));
}

TEST(CheckIfValidNetworkOpCode, FalseOpcodeByNegativNumber) {
    //Technically -1 is some bit positive number since Opcode is an unsigned 8 bit number
    EXPECT_FALSE(hidra2::NetworkProducerPeerImpl::CheckIfValidNetworkOpCode((hidra2::Opcode) - 1));
}

/*
 *
class ConstructorAndGetterFixture : public testing::Test {
 public:

    const std::unique_ptr<hidra2::NetworkProducerPeerImpl> networkProducerPeer;
    virtual void SetUp() {
        networkProducerPeer.reset(new hidra2::NetworkProducerPeerImpl(expected_socket_descriptor, expected_address));
    }
};

 */



}
