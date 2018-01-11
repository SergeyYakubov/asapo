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

TEST(FolderDBConverter, SetCorrectDB) {
    FolderToDbImporter converter{};
    ASSERT_THAT(dynamic_cast<hidra2::MongoDBClient*>(converter.db__.get()), Ne(nullptr));
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
    NiceMock<MockDatabase> mock_db;
    NiceMock<MockIO> mock_io;
    FileInfos file_infos;
    hidra2::ConvertParameters params{};

    void SetUp() override {
        converter.db__ = std::unique_ptr<Database> {&mock_db};
        converter.io__ = std::unique_ptr<IO> {&mock_io};
        file_infos = CreateTestFileInfos();
        params.folder = {"folder"};
        params.uri = "db_address";
        params.ignore_duplicates = true;


        ON_CALL(mock_db, Connect(_, _, _))
        .WillByDefault(Return(DBError::kNoError));
        ON_CALL(mock_io, FilesInFolder(_, _)).
        WillByDefault(DoAll(testing::SetArgPointee<1>(IOError::kNoError),
                            testing::Return(file_infos)));
    }
    void TearDown() override {
        converter.db__.release();
        converter.io__.release();

    }
};


TEST_F(FolderDBConverterTests, ErrorWhenCannotConnect) {


    EXPECT_CALL(mock_db, Connect(params.uri, kDBName, _)).
    WillOnce(testing::Return(DBError::kConnectionError));

    auto error = converter.Convert(params);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kDBConnectionError));
}

TEST_F(FolderDBConverterTests, DBDestructorCalled) {
    mock_db.check_destructor = true;
    EXPECT_CALL(mock_db, Die());
}

TEST_F(FolderDBConverterTests, ErrorWhenCannotGetFileList) {


    EXPECT_CALL(mock_io, FilesInFolder(params.folder, _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(IOError::kReadError),
                   testing::Return(FileInfos {})));

    auto error = converter.Convert(params);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kIOError));

}


TEST_F(FolderDBConverterTests, ErrorWhenCannotImportFileListToDb) {

    EXPECT_CALL(mock_db, Insert(_, _)).
    WillOnce(testing::Return(DBError::kInsertError));

    auto error = converter.Convert(params);
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
        EXPECT_CALL(mock_db, Insert(CompareFileInfo(file), _)).
        WillOnce(testing::Return(DBError::kNoError));
    }

    auto error = converter.Convert(params);
    ASSERT_THAT(error, Eq(FolderToDbImportError::kOK));

}

TEST_F(FolderDBConverterTests, ComputesStatistics) {

    for (auto& file : file_infos) {
        EXPECT_CALL(mock_db, Insert(CompareFileInfo(file), true)).
        WillOnce(testing::Return(DBError::kNoError));
    }

    hidra2::FolderImportStatistics statistics;
    auto error = converter.Convert(params, &statistics);

    ASSERT_THAT(error, Eq(FolderToDbImportError::kOK));
    ASSERT_THAT(statistics.n_files_converted, Eq(file_infos.size()));
// tests may fail is function call is smaller than 1 ns
    ASSERT_THAT(statistics.time_read_folder.count(), Gt(0));
    ASSERT_THAT(statistics.time_import_files.count(), Gt(0));
}

}
