#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"

#include "../../src/file_processors/write_file_processor.h"
#include "common/networking.h"
#include "preprocessor/definitions.h"
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
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;
using asapo::Request;
using asapo::WriteFileProcessor;
using ::asapo::GenericRequestHeader;
using asapo::MockRequest;

namespace {

TEST(WriteFileProcessor, Constructor) {
    WriteFileProcessor processor;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(processor.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(processor.log__), Ne(nullptr));

}

class WriteFileProcessorTests : public Test {
  public:
    WriteFileProcessor processor;
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequest> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_file_name = "2";
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_beamline = "beamline";
    std::string expected_facility = "facility";
    std::string expected_year = "2020";
    uint64_t expected_file_size = 10;
    bool expected_overwrite = false;
    std::string expected_root_folder = "root_folder";
    std::string expected_full_path =  expected_root_folder + asapo::kPathSeparator + expected_facility +
                                      asapo::kPathSeparator + "gpfs" +
                                      asapo::kPathSeparator + expected_beamline +
                                      asapo::kPathSeparator + expected_year +
                                      asapo::kPathSeparator + "data" +
                                      asapo::kPathSeparator + expected_beamtime_id;
    void ExpectFileWrite(const asapo::SimpleErrorTemplate* error_template);
    void MockRequestData(int times = 1);
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        asapo::ReceiverConfig test_config;
        asapo::SetReceiverConfig(test_config, "none");
        processor.log__ = &mock_logger;
        mock_request.reset(new MockRequest{request_header, 1, "", nullptr});
        processor.io__ = std::unique_ptr<asapo::IO> {&mock_io};
    }
    void TearDown() override {
        processor.io__.release();
    }

};

TEST_F(WriteFileProcessorTests, ErrorWhenZeroFileSize) {
    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(0));

    auto err = processor.ProcessFile(mock_request.get(), false);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
}

void WriteFileProcessorTests::MockRequestData(int times) {
    EXPECT_CALL(*mock_request, GetDataSize()).Times(times)
    .WillRepeatedly(Return(expected_file_size));

    EXPECT_CALL(*mock_request, GetData()).Times(times)
    .WillRepeatedly(Return(nullptr));

    EXPECT_CALL(*mock_request, GetOfflinePath()).Times(times)
    .WillRepeatedly(ReturnRef(expected_full_path));

    EXPECT_CALL(*mock_request, GetFileName()).Times(times)
    .WillRepeatedly(Return(expected_file_name));
}

void WriteFileProcessorTests::ExpectFileWrite(const asapo::SimpleErrorTemplate* error_template) {
    EXPECT_CALL(mock_io, WriteDataToFile_t(expected_full_path, expected_file_name, _, expected_file_size, true,
                                           expected_overwrite))
    .WillOnce(
        Return(error_template == nullptr ? nullptr : error_template->Generate().release()));
}

TEST_F(WriteFileProcessorTests, CallsWriteFile) {
    MockRequestData();

    ExpectFileWrite(&asapo::IOErrorTemplates::kUnknownIOError);

    auto err = processor.ProcessFile(mock_request.get(), expected_overwrite);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(WriteFileProcessorTests, WritesToLog) {

    MockRequestData();

    ExpectFileWrite(nullptr);

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("saved file"),
                                         HasSubstr(expected_file_name),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(expected_facility),
                                         HasSubstr(expected_year),
                                         HasSubstr(std::to_string(expected_file_size))
                                        )
                                  )
               );
    auto err = processor.ProcessFile(mock_request.get(), expected_overwrite);
    ASSERT_THAT(err, Eq(nullptr));
}




}
