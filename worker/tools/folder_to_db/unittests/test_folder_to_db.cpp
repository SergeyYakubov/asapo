#include <gmock/gmock.h>
#include "gtest/gtest.h"

#include "system_wrappers/io.h"
#include "system_wrappers/system_io.h"


#include "database/database.h"
#include "database/mongo_database.h"

#include "common/file_info.h"

#include "../src/FolderDBConverter.h"

#include "unittests/MockIO.h"


using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Test;
using ::testing::_;
using ::testing::Mock;
using ::testing::NiceMock;

using hidra2::FolderDBConverter;
using hidra2::FolderDBConverterError;
using hidra2::Database;
using hidra2::IO;
using hidra2::DBError;
using hidra2::IOError;
using hidra2::kDBName;
using hidra2::FileInfo;
using hidra2::MockIO;


namespace {

TEST(FolderDBConverter, SetCorrectIO) {
    FolderDBConverter converter{};
    ASSERT_THAT(dynamic_cast<hidra2::SystemIO*>(converter.io__.get()), Ne(nullptr));
}

TEST(FolderDBConverter, SetCorrectDB) {
    FolderDBConverter converter{};
    ASSERT_THAT(dynamic_cast<hidra2::MongoDB*>(converter.db__.get()), Ne(nullptr));
}

class MockDatabase : public Database {
  public:
    MOCK_METHOD3(Connect, DBError (const std::string&, const std::string&, const std::string&));

    // stuff to test db destructor is called and avoid "uninteresting call" messages
    MOCK_METHOD0(Die, void());
    virtual ~MockDatabase() override {
        if (check_destructor)
            Die();
    }
    bool check_destructor{false};

};



class FolderDBConverterTests : public Test {
  public:
    FolderDBConverter converter{};
    NiceMock<MockDatabase> mock_db;
    NiceMock<MockIO> mock_io;

    void SetUp() override {
        converter.db__ = std::unique_ptr<Database> {&mock_db};
        converter.io__ = std::unique_ptr<IO> {&mock_io};

    }
    void TearDown() override {
        converter.db__.release();
        converter.io__.release();

    }
};


TEST_F(FolderDBConverterTests, ErrorWhenCannotConnect) {

    std::string uri{"db_address"};


    EXPECT_CALL(mock_db, Connect(uri, kDBName, "")).
    WillOnce(testing::Return(DBError::kConnectionError));

    auto error = converter.Convert(uri, "");
    ASSERT_THAT(error, Eq(FolderDBConverterError::kDBConnectionError));
}

TEST_F(FolderDBConverterTests, DBDestructorCalled) {
    mock_db.check_destructor = true;
    EXPECT_CALL(mock_db, Die());
}

TEST_F(FolderDBConverterTests, ErrorWhenCannotGetFileList) {

    std::string folder{"folder"};

    EXPECT_CALL(mock_io, FilesInFolder(folder, _)).
        WillOnce(DoAll(testing::SetArgPointee<1>(IOError::kReadError), testing::Return(std::vector<FileInfo>{})));


    auto error = converter.Convert("", folder);
    ASSERT_THAT(error, Eq(FolderDBConverterError::kIOError));

}



}
