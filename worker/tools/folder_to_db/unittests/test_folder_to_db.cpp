#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <thread>

#include "system_wrappers/io.h"
#include "system_wrappers/system_io.h"


#include "database/database.h"

#include "common/data_structs.h"

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
using hidra2::IOErrors;
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
    ASSERT_THAT(dynamic_cast<hidra2::DatabaseFactory*>(converter.db_factory__.get()), Ne(nullptr));
}

TEST(FolderDBConverter, SetNTasksCorrectly) {

    unsigned int max_tasks = std::thread::hardware_concurrency();

    FolderToDbImporter converter{};
    auto res = converter.SetNParallelTasks(0);
    ASSERT_THAT(res, Eq(1));

    res = converter.SetNParallelTasks(2);
    ASSERT_THAT(res, Eq(2));

    res = converter.SetNParallelTasks(1000000);
    ASSERT_THAT(res, Eq(max_tasks));

}

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

class MockDatabaseFactory : public DatabaseFactory {
  public:
    std::vector<NiceMock<MockDatabase>*> db;
    mutable int n{0};
    void CreateDBs(int n) {
        for (int i = 0; i < n; i++) {
            auto val = new NiceMock<MockDatabase>;
            db.push_back(val);
            ON_CALL(*val, Connect(_, _, _))
            .WillByDefault(Return(DBError::kNoError));
        }
    }
    std::unique_ptr<Database> Create(DBError* err) const noexcept override {
        *err = DBError::kNoError;
        return std::unique_ptr<Database> {db[n++]};
    }
    ~MockDatabaseFactory() {
        for (unsigned int i = n; i < db.size(); i++) {
            delete db[i];
        }
    }
};

class FakeDatabaseFactory : public DatabaseFactory {
    std::unique_ptr<Database> Create(DBError* err) const noexcept override {
        *err = DBError::kMemoryError;
        return {};
    }
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

    MockDatabaseFactory* mock_dbf;
    FileInfos file_infos;
    std::string folder, uri;
    void SetUp() override {
        converter.io__ = std::unique_ptr<IO> {&mock_io};
        mock_dbf = new MockDatabaseFactory;
        mock_dbf->CreateDBs(3);
        converter.db_factory__ = std::unique_ptr<DatabaseFactory> {mock_dbf};
        file_infos = CreateTestFileInfos();
        folder = "folder";
        uri = "db_address";
        ON_CALL(mock_io, FilesInFolder(_, _)).
        WillByDefault(DoAll(testing::SetArgPointee<1>(IOErrors::kNoError),
                            testing::Return(file_infos)));
    }
    void TearDown() override {
        converter.io__.release();
    }
};


TEST_F(FolderDBConverterTests, ErrorWhenCannotConnect) {
    EXPECT_CALL(*(mock_dbf->db[0]), Connect(uri, kDBName, _)).
    WillOnce(testing::Return(DBError::kConnectionError));

    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kDBConnectionError));
}

TEST_F(FolderDBConverterTests, ErrorWhenCannotCreateDbParallel) {
    int nparallel = 3;
    EXPECT_CALL(*(mock_dbf->db[0]), Connect(uri, kDBName, _)).
    WillOnce(testing::Return(DBError::kConnectionError));
    EXPECT_CALL(*(mock_dbf->db[1]), Connect(uri, kDBName, _)).
    WillOnce(testing::Return(DBError::kConnectionError));
    EXPECT_CALL(*(mock_dbf->db[2]), Connect(uri, kDBName, _)).
    WillOnce(testing::Return(DBError::kConnectionError));

    converter.SetNParallelTasks(nparallel);
    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kDBConnectionError));
}


TEST_F(FolderDBConverterTests, DBDestructorCalled) {
    mock_dbf->db[0]->check_destructor = true;
    EXPECT_CALL(*(mock_dbf->db[0]), Die());
}

TEST_F(FolderDBConverterTests, ErrorWhenCannotGetFileList) {


    EXPECT_CALL(mock_io, FilesInFolder(folder, _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(IOErrors::kReadError),
                   testing::Return(FileInfos {})));

    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kIOError));
}

TEST_F(FolderDBConverterTests, PassesIgnoreDuplicates) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert(_, true));

    converter.IgnoreDuplicates(true);
    converter.Convert(uri, folder);
}



TEST_F(FolderDBConverterTests, ErrorWhenCannotImportFileListToDb) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert(_, _)).
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
        EXPECT_CALL(*(mock_dbf->db[0]), Insert(CompareFileInfo(file), _)).
        WillOnce(testing::Return(DBError::kNoError));
    }

    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kOK));

}

TEST_F(FolderDBConverterTests, PassesFileListToInsertInParallel3by3) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert(CompareFileInfo(file_infos[0]), _)).
    WillOnce(testing::Return(DBError::kNoError));
    EXPECT_CALL(*(mock_dbf->db[1]), Insert(CompareFileInfo(file_infos[1]), _)).
    WillOnce(testing::Return(DBError::kNoError));
    EXPECT_CALL(*(mock_dbf->db[2]), Insert(CompareFileInfo(file_infos[2]), _)).
    WillOnce(testing::Return(DBError::kNoError));

    converter.SetNParallelTasks(3, false);
    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kOK));
}

TEST_F(FolderDBConverterTests, PassesFileListToInsertInParallel3by2) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert(CompareFileInfo(file_infos[0]), _)).
    WillOnce(testing::Return(DBError::kNoError));
    EXPECT_CALL(*(mock_dbf->db[0]), Insert(CompareFileInfo(file_infos[1]), _)).
    WillOnce(testing::Return(DBError::kNoError));
    EXPECT_CALL(*(mock_dbf->db[1]), Insert(CompareFileInfo(file_infos[2]), _)).
    WillOnce(testing::Return(DBError::kNoError));

    converter.SetNParallelTasks(2, false);
    auto error = converter.Convert(uri, folder);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kOK));
}

TEST_F(FolderDBConverterTests, ComputesStatistics) {

    EXPECT_CALL(*mock_dbf->db[0], Insert(_, false)).
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


TEST_F(FolderDBConverterTests, ErrorWhenCannotCreateDB) {
    converter.db_factory__ = std::unique_ptr<DatabaseFactory> {new FakeDatabaseFactory};

    auto err = converter.Convert("", "");

    ASSERT_THAT(err, Eq(FolderToDbImportError::kMemoryError));

}


}
