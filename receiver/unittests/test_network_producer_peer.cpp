#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/receiver.h"

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Mock;
using ::testing::InSequence;

namespace {

TEST(CheckIfValidFileSize, ZeroFileSize) {
    EXPECT_FALSE(hidra2::NetworkProducerPeer::CheckIfValidFileSize(0));
}

TEST(CheckIfValidFileSize, SmallFileSize) {
    EXPECT_TRUE(hidra2::NetworkProducerPeer::CheckIfValidFileSize(1));
}

TEST(CheckIfValidFileSize, OneGiByteSize) {
    EXPECT_TRUE(hidra2::NetworkProducerPeer::CheckIfValidFileSize(1024 * 1024 * 1024 * 1));
}

TEST(CheckIfValidFileSize, TwoGiByteSize) {
    EXPECT_TRUE(hidra2::NetworkProducerPeer::CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2));
}

TEST(CheckIfValidFileSize, MoreThenTwoGiByteSize) {
    EXPECT_FALSE(hidra2::NetworkProducerPeer::CheckIfValidFileSize(size_t(1024) * 1024 * 1024 * 2 + 1));
}

TEST(CreateAndOpenFileByFileId, FolderUnknownIOError) {
    hidra2::NetworkProducerPeer networkProducerPeer;
    hidra2::MockIO mockIO;
    networkProducerPeer.__set_io(&mockIO);

    uint64_t expected_file_id = 5435452453867;

    hidra2::Error err;
    EXPECT_CALL(mockIO, CreateNewDirectory_t("files", _))
    .Times(1)
    .WillOnce(
        testing::SetArgPointee<1>(hidra2::IOErrorTemplates::kUnknownIOError.Generate().release())
    );
    networkProducerPeer.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST(CreateAndOpenFileByFileId, FolderAlreadyExsistsButFileAlreadyExists) {
    hidra2::NetworkProducerPeer networkProducerPeer;
    hidra2::MockIO mockIO;
    networkProducerPeer.__set_io(&mockIO);

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

    hidra2::FileDescriptor fd = networkProducerPeer.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kFileAlreadyExists));
    ASSERT_THAT(fd, Eq(expected_fd));
}

TEST(CreateAndOpenFileByFileId, FolderCreatedButFileAlreadyExists) {
    hidra2::NetworkProducerPeer networkProducerPeer;
    hidra2::MockIO mockIO;
    networkProducerPeer.__set_io(&mockIO);

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

    hidra2::FileDescriptor fd = networkProducerPeer.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kFileAlreadyExists));
    ASSERT_THAT(fd, Eq(-1));
}

TEST(CreateAndOpenFileByFileId, Ok) {
    hidra2::NetworkProducerPeer networkProducerPeer;
    hidra2::MockIO mockIO;
    networkProducerPeer.__set_io(&mockIO);

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

    hidra2::FileDescriptor fd = networkProducerPeer.CreateAndOpenFileByFileId(expected_file_id, &err);
    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(fd, Eq(expected_fd));
}

}
