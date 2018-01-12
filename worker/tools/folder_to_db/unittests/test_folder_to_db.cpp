#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "system_wrappers/io.h"
#include "system_wrappers/system_io.h"


#include "database/database.h"
#include "database/mongodb_client.h"

#include "common/file_info.h"

#include "../src/folder_db_importer.h"

#include "unittests/MockIO.h"


using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Ref;
using ::testing::Return;

using hidra2::FolderToDbImporter;
using hidra2::FolderToDbImportError;
using hidra2::Database;
using hidra2::DatabaseFactory;
using hidra2::IO;
using hidra2::DBError;
using hidra2::IOError;
using hidra2::kDBName;
using hidra2::FileInfos;
using hidra2::FileInfo;
using hidra2::MockIO;


namespace {

TEST(FolderDBConverter, SetCorrectIO) {
    FolderToDbImporter converter{};
    ASSERT_THAT(dynamic_cast<hidra2::SystemIO*>(converter.io__.get()), Ne(nullptr));
}

TEST(FolderDBConverter, SetCorrectDBFactory) {
    FolderToDbImporter converter{};
    ASSERT_THAT(dynamic_cast<hidra2::MongoDatabaseFactory*>(converter.db_factory__.get()), Ne(nullptr));
}

class MockDatabaseFactory : public DatabaseFactory {
  public:
    Database* db_;
    std::unique_ptr<Database> Create(DBError* err) const noexcept {
        return std::unique_ptr<Database> {db_};
    }
};


class MockDatabase : public Database {
  public:
    MOCK_METHOD3(Connect, DBError (const std::string&, const std::string&, const std::string&));
    MOCK_CONST_METHOD2(Insert, DBError (const FileInfo&, bool));

    // stuff to test db destructor is called and avoid "uninteresting call" messages
    MOCK_METHOD0(Die, void());
    virtual ~MockDatabase() override {
        if (check_destructor)
            Die();
    }
    bool check_destructor{false};
};

FileInfos CreateTestFileInfos() {
    FileInfos file_infos;
    FileInfo fi;
    fi.size = 100;
    fi.base_name = "1";
    file_infos.push_back(fi);
    fi.base_name = "2";
    file_infos.push_back(fi);
    fi.base_name = "3";
    file_infos.push_back(fi);
    return file_infos;
}

class FolderDBConverterTests : public Test {
  public:
    FolderToDbImporter converter{};
    NiceMock<MockIO> mock_io;
    NiceMock<MockDatabase>* mock_db = new NiceMock<MockDatabase>;
    MockDatabaseFactory* mock_dbf = new MockDatabaseFactory;
    FileInfos file_infos;
    std::string folder, uri;
    void SetUp() override {
        mock_dbf->db_ = mock_db;
        converter.io__ = std::unique_ptr<IO> {&mock_io};
        converter.db_factory__ = std::unique_ptr<DatabaseFactory> {mock_dbf};
        file_infos = CreateTestFileInfos();
        folder = "folder";
        uri = "db_address";

        ON_CALL(*mock_db, Connect(_, _, _))
        .WillByDefault(Return(DBError::kNoError));
        ON_CALL(mock_io, FilesInFolder(_, _)).
        WillByDefault(DoAll(testing::SetArgPointee<1>(IOError::kNoError),
                            testing::Return(file_infos)));
    }
    void TearDown() override {
        converter.io__.release();
    }
};

TEST_F(FolderDBConverterTests, ErrorWhenCannotConnect) {
    EXPECT_CALL(*mock_db, Connect(uri, kDBName, _)).
    WillOnce(testing::Return(DBError::kConnectionError));

    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kDBConnectionError));
}


TEST_F(FolderDBConverterTests, DBDestructorCalled) {
    mock_db->check_destructor = true;
    EXPECT_CALL(*mock_db, Die());
    delete mock_db;
}

TEST_F(FolderDBConverterTests, ErrorWhenCannotGetFileList) {


    EXPECT_CALL(mock_io, FilesInFolder(folder, _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(IOError::kReadError),
                   testing::Return(FileInfos {})));

    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kIOError));
    delete mock_db;

}

TEST_F(FolderDBConverterTests, PassesIgnoreDuplicates) {

    EXPECT_CALL(*mock_db, Insert(_, true));

    converter.IgnoreDuplicates(true);
    converter.Convert(uri, folder);
    converter.IgnoreDuplicates(false);

}

TEST_F(FolderDBConverterTests, ErrorWhenCannotImportFileListToDb) {

    EXPECT_CALL(*mock_db, Insert(_, _)).
    WillOnce(testing::Return(DBError::kInsertError));

    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kImportError));

}
// a matcher to compare file_infos (size and basename only) for testing purposes
// (we do not want to create an == operator for FileInfo)
MATCHER_P(CompareFileInfo, file, "") {
    if (arg.size != file.size) return false;
    if (arg.base_name != file.base_name) return false;
    return true;
}


TEST_F(FolderDBConverterTests, PassesFileListToInsert) {

    for (auto& file : file_infos) {
        EXPECT_CALL(*mock_db, Insert(CompareFileInfo(file), _)).
        WillOnce(testing::Return(DBError::kNoError));
    }

    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kOK));

}

TEST_F(FolderDBConverterTests, ComputesStatistics) {

    EXPECT_CALL(*mock_db, Insert(_, false)).
    Times(file_infos.size()).
    WillRepeatedly(testing::Return(DBError::kNoError));

    hidra2::FolderImportStatistics statistics;
    auto error = converter.Convert(uri, folder, &statistics);

    ASSERT_THAT(error, Eq(FolderToDbImportError::kOK));
    ASSERT_THAT(statistics.n_files_converted, Eq(file_infos.size()));
// tests may fail is function call is smaller than 1 ns
    ASSERT_THAT(statistics.time_read_folder.count(), Gt(0));
    ASSERT_THAT(statistics.time_import_files.count(), Gt(0));
}

}
