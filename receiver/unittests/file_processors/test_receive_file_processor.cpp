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
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;
using asapo::Request;
using asapo::ReceiveFileProcessor;
using ::asapo::GenericRequestHeader;
using asapo::MockRequest;

namespace {

TEST(ReceiveFileProcessor, Constructor) {
    ReceiveFileProcessor processor;
    ASSERT_THAT(dynamic_cast<asapo::IO*>(processor.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(processor.log__), Ne(nullptr));

}

class ReceiveFileProcessorTests : public Test {
  public:
    ReceiveFileProcessor processor;
    NiceMock<MockIO> mock_io;
    std::unique_ptr<MockRequest> mock_request;
    NiceMock<asapo::MockLogger> mock_logger;
    SocketDescriptor expected_socket_id = SocketDescriptor{1};
    std::string expected_file_name = std::string("processed")+asapo::kPathSeparator+std::string("2");
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_beamline = "beamline";
    std::string expected_facility = "facility";
    std::string expected_year = "2020";
    asapo::SourceType expected_source_type = asapo::SourceType::kProcessed;
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
    void MockRequestData();
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

void ReceiveFileProcessorTests::MockRequestData() {

    EXPECT_CALL(*mock_request, GetSocket())
    .WillOnce(Return(expected_socket_id))
    ;

    EXPECT_CALL(*mock_request, GetDataSize()).Times(1)
    .WillRepeatedly(Return(expected_file_size));

    EXPECT_CALL(*mock_request, GetOfflinePath()).Times(1)
    .WillRepeatedly(ReturnRef(expected_full_path));

    EXPECT_CALL(*mock_request, GetSourceType()).Times(2)
        .WillRepeatedly(Return(expected_source_type));


    EXPECT_CALL(*mock_request, GetFileName()).Times(2)
    .WillRepeatedly(Return(expected_file_name));
}

void ReceiveFileProcessorTests::ExpectFileWrite(const asapo::SimpleErrorTemplate* error_template) {
    EXPECT_CALL(mock_io, WriteDataToFile_t(expected_full_path, expected_file_name, _, expected_file_size, true,
                                           expected_overwrite))
    .WillOnce(
        Return(error_template == nullptr ? nullptr : error_template->Generate().release()));
}

TEST_F(ReceiveFileProcessorTests, CallsReceiveFile) {
    asapo::ReceiverConfig test_config;

    asapo::SetReceiverConfig(test_config, "none");

    MockRequestData();

    EXPECT_CALL(mock_io, ReceiveDataToFile_t(expected_socket_id, expected_full_path, expected_file_name, expected_file_size,
                                             true, expected_overwrite))
    .WillOnce(
        Return(asapo::IOErrorTemplates::kUnknownIOError.Generate().release())
    );

    auto err = processor.ProcessFile(mock_request.get(), expected_overwrite);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}


TEST_F(ReceiveFileProcessorTests, WritesToLog) {

    MockRequestData();

    EXPECT_CALL(mock_io, ReceiveDataToFile_t(_, _, _, _, _, _))
    .WillOnce(Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("received file"),
                                         HasSubstr(expected_file_name),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(std::to_string(expected_file_size))
                                        )
                                  )
               );
    processor.ProcessFile(mock_request.get(), expected_overwrite);
}



}
