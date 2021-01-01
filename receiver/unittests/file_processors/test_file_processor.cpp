#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/file_processors/receive_file_processor.h"
#include "asapo/common/networking.h"
#include "asapo/preprocessor/definitions.h"
#include "../mock_receiver_config.h"

#include "../receiver_mocking.h"

using ::testing::Test;
using ::testing::Return;
using ::testing::ReturnRef;
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
using ::testing::AllOf;
using ::testing::HasSubstr;


using ::asapo::Error;
using ::asapo::GetRootFolder;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;
using asapo::Request;
using asapo::ReceiveFileProcessor;
using ::asapo::GenericRequestHeader;
using asapo::MockRequest;

namespace {

class FileProcessorTests : public Test {
  public:
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequest> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_offline_path =  "offline";
    std::string expected_online_path =  "online";
    void MockRequestData(std::string fname,asapo::SourceType type);
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        asapo::ReceiverConfig test_config;
        asapo::SetReceiverConfig(test_config, "none");
        mock_request.reset(new MockRequest{request_header, 1, "", nullptr});
    }
    void TearDown() override {
    }

};

void FileProcessorTests::MockRequestData(std::string fname,asapo::SourceType type) {

    if (type == asapo::SourceType::kProcessed) {
            EXPECT_CALL(*mock_request, GetOfflinePath())
             .WillRepeatedly(ReturnRef(expected_offline_path));
    } else {
        EXPECT_CALL(*mock_request, GetOnlinePath())
            .WillRepeatedly(ReturnRef(expected_online_path));
    }

    EXPECT_CALL(*mock_request, GetSourceType()).WillRepeatedly(Return(type));

    EXPECT_CALL(*mock_request, GetFileName()).Times(1)
    .WillRepeatedly(Return(fname));
}


std::string repl_sep(const std::string& orig) {
    std::string str = orig;
    std::replace(str.begin(), str.end(), '/', asapo::kPathSeparator); // needed for Windows tests
    return str;
}

TEST_F(FileProcessorTests, RawWriteToRaw) {

    struct Test {
      asapo::SourceType type;
      std::string filename;
      bool error;
      std::string res;
    };
    std::vector<Test> tests = {
        Test{asapo::SourceType::kProcessed,repl_sep("processed/bla.text"),false,expected_offline_path},
        Test{asapo::SourceType::kProcessed,repl_sep("raw/bla.text"),true,""},
        Test{asapo::SourceType::kProcessed,repl_sep("processed/../bla.text"),true,""},
        Test{asapo::SourceType::kProcessed,repl_sep("bla/bla.text"),true,""},
        Test{asapo::SourceType::kProcessed,repl_sep("bla.text"),true,""},
        Test{asapo::SourceType::kProcessed,repl_sep("./bla.text"),true,""},
        Test{asapo::SourceType::kRaw,repl_sep("raw/bla.text"),false,expected_online_path},
    };

    for (auto& test: tests) {
        MockRequestData(test.filename,test.type);
        std::string res;
        auto err = GetRootFolder(mock_request.get(),&res);
        if (test.error) {
            ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
        } else {
            ASSERT_THAT(err, Eq(nullptr));
            ASSERT_THAT(res, Eq(test.res));
        }
        Mock::VerifyAndClearExpectations(&mock_request);
    }

}



}
