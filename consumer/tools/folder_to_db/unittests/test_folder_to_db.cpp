#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <thread>

#include "io/io.h"
#include "../../../../common/cpp/src/system_io/system_io.h"

#include "database/db_error.h"


#include "database/database.h"

#include "common/data_structs.h"
#include "unittests/MockDatabase.h"

#include "../src/folder_db_importer.h"
#include "database/db_error.h"

#include "unittests/MockIO.h"


using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Ge;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Ref;
using ::testing::Return;
using ::testing::DoAll;

using namespace asapo;


namespace {


TEST(FolderDBConverter, SetCorrectIO) {
    FolderToDbImporter converter{};
    ASSERT_THAT(dynamic_cast<SystemIO*>(converter.io__.get()), Ne(nullptr));
}

TEST(FolderDBConverter, SetCorrectDBFactory) {
    FolderToDbImporter converter{};
    ASSERT_THAT(dynamic_cast<DatabaseFactory*>(converter.db_factory__.get()), Ne(nullptr));
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


class MockDatabaseFactory : public DatabaseFactory {
  public:
    std::vector<NiceMock<MockDatabase>*> db;
    mutable int n{0};
    void CreateDBs(int n) {
        for (int i = 0; i < n; i++) {
            auto val = new NiceMock<MockDatabase>;
            db.push_back(val);
            ON_CALL(*val, Connect_t(_, _))
            .WillByDefault(Return(nullptr));
        }
    }
    std::unique_ptr<Database> Create(Error* err) const noexcept override {
        *err = nullptr;
        return std::unique_ptr<Database> {db[n++]};
    }
    ~MockDatabaseFactory() {
        for (unsigned int i = n; i < db.size(); i++) {
            delete db[i];
        }
    }
};

class FakeDatabaseFactory : public DatabaseFactory {
    std::unique_ptr<Database> Create(Error* err) const noexcept override {
        *err = asapo::ErrorTemplates::kMemoryAllocationError.Generate();
        return {};
    }
};

FileInfos CreateTestFileInfos() {
    FileInfos file_infos;
    FileInfo fi;
    fi.size = 100;
    fi.name = "1";
    file_infos.push_back(fi);
    fi.name = "2";
    file_infos.push_back(fi);
    fi.name = "3";
    file_infos.push_back(fi);
    return file_infos;
}

class FolderDBConverterTests : public Test {
  public:
    FolderToDbImporter converter{};
    NiceMock<MockIO> mock_io;
    std::string expected_collection_name = std::string(kDBDataCollectionNamePrefix) + "_default";
    MockDatabaseFactory* mock_dbf;
    FileInfos file_infos;
    std::string folder, uri, db_name;
    void SetUp() override {
        converter.io__ = std::unique_ptr<IO> {&mock_io};
        mock_dbf = new MockDatabaseFactory;
        mock_dbf->CreateDBs(3);
        converter.db_factory__ = std::unique_ptr<DatabaseFactory> {mock_dbf};
        file_infos = CreateTestFileInfos();
        folder = "folder";
        db_name = "db_name";
        uri = "db_address";
        ON_CALL(mock_io, FilesInFolder_t(_, _)).
        WillByDefault(DoAll(testing::SetArgPointee<1>(nullptr),
                            testing::Return(file_infos)));
    }
    void TearDown() override {
        converter.io__.release();
    }
};

TEST_F(FolderDBConverterTests, ErrorWhenCannotConnect) {
    EXPECT_CALL(*(mock_dbf->db[0]), Connect_t(uri, db_name)).
    WillOnce(testing::Return(asapo::DBErrorTemplates::kConnectionError.Generate().release()));

    auto error = converter.Convert(uri, folder, db_name);
    ASSERT_THAT(error, Ne(nullptr));
}

TEST_F(FolderDBConverterTests, ErrorWhenCannotCreateDbParallel) {
    int nparallel = 3;
    EXPECT_CALL(*(mock_dbf->db[0]), Connect_t(uri, _)).
    WillOnce(testing::Return(asapo::DBErrorTemplates::kConnectionError.Generate().release()));
    EXPECT_CALL(*(mock_dbf->db[1]), Connect_t(uri, _)).
    WillOnce(testing::Return(asapo::DBErrorTemplates::kConnectionError.Generate().release()));
    EXPECT_CALL(*(mock_dbf->db[2]), Connect_t(uri, _)).
    WillOnce(testing::Return(asapo::DBErrorTemplates::kConnectionError.Generate().release()));

    converter.SetNParallelTasks(nparallel);
    auto error = converter.Convert(uri, folder, db_name);
    ASSERT_THAT(error, Ne(nullptr));
}


TEST_F(FolderDBConverterTests, DBDestructorCalled) {
    mock_dbf->db[0]->check_destructor = true;
    EXPECT_CALL(*(mock_dbf->db[0]), Die());
}

TEST_F(FolderDBConverterTests, ErrorWhenCannotGetFileList) {


    EXPECT_CALL(mock_io, FilesInFolder_t(folder, _)).
    WillOnce(DoAll(testing::SetArgPointee<1>(new asapo::SimpleError("err")),
                   testing::Return(FileInfos {})));

    auto error = converter.Convert(uri, folder, db_name);
    ASSERT_THAT(error, Ne(nullptr));
}



TEST_F(FolderDBConverterTests, PassesIgnoreDuplicates) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert_t(expected_collection_name, _, true)).Times(3);

    converter.IgnoreDuplicates(true);
    converter.Convert(uri, folder, db_name);
}


TEST_F(FolderDBConverterTests, ErrorWhenCannotImportFileListToDb) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert_t(_, _, _)).
    WillOnce(testing::Return(asapo::DBErrorTemplates::kInsertError.Generate().release()));

    auto error = converter.Convert(uri, folder, db_name);
    ASSERT_THAT(error, Ne(nullptr));

}
// a matcher to compare file_infos (size and basename only) for testing purposes
// (we do not want to create an == operator for FileInfo)
MATCHER_P(CompareFileInfo, file, "") {
    if (arg.size != file.size) return false;
    if (arg.name != file.name) return false;
    return true;
}


TEST_F(FolderDBConverterTests, PassesFileListToInsert) {

    for (auto& file : file_infos) {
        EXPECT_CALL(*(mock_dbf->db[0]), Insert_t(expected_collection_name, CompareFileInfo(file), _)).
        WillOnce(testing::Return(nullptr));
    }

    auto error = converter.Convert(uri, folder, db_name);
    ASSERT_THAT(error, Eq(nullptr));

}

TEST_F(FolderDBConverterTests, PassesFileListToInsertInParallel3by3) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert_t(expected_collection_name, CompareFileInfo(file_infos[0]), _)).
    WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*(mock_dbf->db[1]), Insert_t(expected_collection_name, CompareFileInfo(file_infos[1]), _)).
    WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*(mock_dbf->db[2]), Insert_t(expected_collection_name, CompareFileInfo(file_infos[2]), _)).
    WillOnce(testing::Return(nullptr));

    converter.SetNParallelTasks(3, false);
    auto error = converter.Convert(uri, folder, db_name);
    ASSERT_THAT(error, Eq(nullptr));
}

TEST_F(FolderDBConverterTests, PassesFileListToInsertInParallel3by2) {

    EXPECT_CALL(*(mock_dbf->db[0]), Insert_t(expected_collection_name, CompareFileInfo(file_infos[0]), _)).
    WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*(mock_dbf->db[0]), Insert_t(expected_collection_name, CompareFileInfo(file_infos[1]), _)).
    WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*(mock_dbf->db[1]), Insert_t(expected_collection_name, CompareFileInfo(file_infos[2]), _)).
    WillOnce(testing::Return(nullptr));

    converter.SetNParallelTasks(2, false);
    auto error = converter.Convert(uri, folder, db_name);
    ASSERT_THAT(error, Eq(nullptr));
}

TEST_F(FolderDBConverterTests, ComputesStatistics) {

    EXPECT_CALL(*mock_dbf->db[0], Insert_t(_, _, false)).
    Times(file_infos.size()).
    WillRepeatedly(testing::Return(nullptr));

    asapo::FolderImportStatistics statistics;

    statistics.time_read_folder = std::chrono::nanoseconds{ -1};
    statistics.time_import_files = std::chrono::nanoseconds{ -1};

    auto error = converter.Convert(uri, folder, db_name, &statistics);

    ASSERT_THAT(error, Eq(nullptr));
    ASSERT_THAT(statistics.n_files_converted, Eq(file_infos.size()));
    ASSERT_THAT(statistics.time_read_folder.count(), Ge(0));
    ASSERT_THAT(statistics.time_import_files.count(), Ge(0));
}


TEST_F(FolderDBConverterTests, ErrorWhenCannotCreateDB) {
    converter.db_factory__ = std::unique_ptr<DatabaseFactory> {new FakeDatabaseFactory};

    auto err = converter.Convert("", "", "");

    ASSERT_THAT(err, Ne(nullptr));

}


}
