#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockLogger.h"

#include "../../../src/request_handler/file_processors/write_file_processor.h"
#include "asapo/common/networking.h"
#include "asapo/preprocessor/definitions.h"
#include "../../mock_receiver_config.h"

#include "../../receiver_mocking.h"

using namespace testing;
using namespace asapo;

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
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    std::string expected_file_name = std::string("raw") + asapo::kPathSeparator + std::string("2");
    asapo::SourceType expected_source_type = asapo::SourceType::kRaw;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_beamline = "beamline";
    std::string expected_facility = "facility";
    std::string expected_year = "2020";
    uint64_t expected_file_size = 10;
    bool expected_overwrite = false;
    std::string expected_root_folder = "root_folder";
    CustomRequestData expected_custom_data {kDefaultIngestMode, 0, 0};
    std::string expected_full_path =  expected_root_folder + asapo::kPathSeparator + expected_facility +
                                      asapo::kPathSeparator + "gpfs" +
                                      asapo::kPathSeparator + expected_beamline +
                                      asapo::kPathSeparator + expected_year +
                                      asapo::kPathSeparator + "data" +
                                      asapo::kPathSeparator + expected_beamtime_id;
    void ExpectFileWrite(const asapo::ErrorTemplateInterface* error_template);
    void MockRequestData(int times = 1);
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 2;
        asapo::ReceiverConfig test_config;
        asapo::SetReceiverConfig(test_config, "none");
        processor.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest>{request_header, 1, "", nullptr});
        processor.io__ = std::unique_ptr<asapo::IO> {&mock_io};
        SetDefaultRequestCalls(mock_request.get(),expected_beamtime_id);

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

    EXPECT_CALL(*mock_request, GetOnlinePath()).Times(times)
    .WillRepeatedly(ReturnRef(expected_full_path));

    EXPECT_CALL(*mock_request, GetSourceType()).Times(times * 2)
    .WillRepeatedly(Return(expected_source_type));

    EXPECT_CALL(*mock_request, GetCustomData_t()).WillRepeatedly(Return(expected_custom_data));


    EXPECT_CALL(*mock_request, GetFileName()).Times(times * 2)
    .WillRepeatedly(Return(expected_file_name));
}

void WriteFileProcessorTests::ExpectFileWrite(const asapo::ErrorTemplateInterface* error_template) {
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

    EXPECT_CALL(mock_logger, Debug(HasSubstr("saved file")));

    auto err = processor.ProcessFile(mock_request.get(), expected_overwrite);
    ASSERT_THAT(err, Eq(nullptr));
}




}
