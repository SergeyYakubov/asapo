#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <asapo/unittests/MockIO.h>
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler/request_handler.h"
#include "../src/request_handler/request_handler_file_process.h"
#include "../src/request_handler/request_handler_db_write.h"
#include "asapo/database/database.h"

#include "receiver_mocking.h"
#include "mock_receiver_config.h"

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::GenericRequestHeader;
using ::asapo::SendResponse;
using ::asapo::GenericRequestHeader;
using ::asapo::GenericNetworkResponse;
using ::asapo::Opcode;
using ::asapo::Connection;
using ::asapo::MockIO;
using asapo::Request;
using asapo::MockStatistics;
using asapo::MockDataCache;
using asapo::StatisticEntity;

using asapo::ReceiverConfig;
using asapo::SetReceiverConfig;
using asapo::RequestFactory;

namespace {

class MockReqestHandler : public asapo::ReceiverRequestHandler {
  public:
    Error ProcessRequest(Request* request) const override {
        return Error{ProcessRequest_t(*request)};
    }

    StatisticEntity GetStatisticEntity() const override {
        return StatisticEntity::kDisk;
    }

    MOCK_CONST_METHOD1(ProcessRequest_t, ErrorInterface * (const Request& request));

};



TEST(RequestTest, Constructor) {
    std::unique_ptr<Request> request;
    GenericRequestHeader generic_request_header;
    request.reset(new Request{generic_request_header, 1, "", nullptr, nullptr, nullptr});
    ASSERT_THAT(request->WasAlreadyProcessed(), false);
}


class RequestTests : public Test {
  public:
    GenericRequestHeader generic_request_header;
    asapo::SocketDescriptor expected_socket_id{1};
    uint64_t data_size_ {100};
    uint64_t data_id_{15};
    uint64_t expected_slot_id{16};
    std::string expected_origin_uri = "origin_uri";
    std::string expected_metadata = "meta";
    std::string expected_stream = "stream";
    uint64_t expected_metadata_size = expected_metadata.size();
    asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;
    char expected_request_message[asapo::kMaxMessageSize] = "test_message";
    std::string expected_api_version = "v0.2";
    std::unique_ptr<Request> request;
    StrictMock<MockIO> mock_io;
    std::shared_ptr<StrictMock<asapo::MockInstancedStatistics>> mock_instanced_statistics;
    MockDataCache mock_cache;
    void SetUp() override {
        mock_instanced_statistics.reset(new StrictMock<asapo::MockInstancedStatistics>);
        generic_request_header.data_size = data_size_;
        generic_request_header.data_id = data_id_;
        generic_request_header.meta_size = expected_metadata_size;
        generic_request_header.op_code = expected_op_code;
        generic_request_header.custom_data[asapo::kPosIngestMode] = asapo::kDefaultIngestMode;
        strcpy(generic_request_header.message, expected_request_message);
        strcpy(generic_request_header.api_version, expected_api_version.c_str());
        request.reset(new Request{generic_request_header, expected_socket_id, expected_origin_uri, nullptr, nullptr, mock_instanced_statistics });
        request->io__ = std::unique_ptr<asapo::IO> {&mock_io};
        ON_CALL(mock_io, Receive_t(expected_socket_id, _, data_size_, _)).WillByDefault(
            DoAll(SetArgPointee<3>(nullptr),
                  Return(0)
                 ));
    }
    void TearDown() override {
        request->io__.release();
    }
    void ExpectFileName(std::string sended, std::string received);
};


TEST_F(RequestTests, HandleProcessesRequests) {

    MockReqestHandler mock_request_handler;

    EXPECT_CALL(mock_request_handler, ProcessRequest_t(_)).WillOnce(
        Return(nullptr)
    ).WillOnce(
        Return(new asapo::IOError("Test Send Error", asapo::IOErrorType::kUnknownIOError))
    );

    request->AddHandler(&mock_request_handler);
    request->AddHandler(&mock_request_handler);

    EXPECT_CALL(*mock_instanced_statistics, StartTimer(asapo::StatisticEntity::kDisk)).Times(2);

    EXPECT_CALL(*mock_instanced_statistics, StopTimer()).Times(2);

    auto err = request->Handle();

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(RequestTests, DataIsNullAtInit) {
    auto data = request->GetData();
    ASSERT_THAT(data, Eq(nullptr));
}

TEST_F(RequestTests, GetDataID) {
    auto id = request->GetDataID();

    ASSERT_THAT(id, Eq(data_id_));
}

TEST_F(RequestTests, GetOpCode) {
    auto code = request->GetOpCode();

    ASSERT_THAT(code, Eq(expected_op_code));
}


TEST_F(RequestTests, GetRequestMessage) {
    auto message = request->GetMessage();

    ASSERT_THAT(message, testing::StrEq(expected_request_message));
}

TEST_F(RequestTests, GetApiVersion) {
    auto ver = request->GetApiVersion();
    ASSERT_THAT(ver, testing::Eq(expected_api_version));
}


TEST_F(RequestTests, GetOriginUri) {
    auto uri = request->GetOriginUri();

    ASSERT_THAT(uri, Eq(expected_origin_uri));
}


TEST_F(RequestTests, GetDataSize) {
    auto size = request->GetDataSize();

    ASSERT_THAT(size, Eq(data_size_));
}


void RequestTests::ExpectFileName(std::string sended, std::string received) {
    strcpy(generic_request_header.message, sended.c_str());

    request->io__.release();
    request.reset(new Request{generic_request_header, expected_socket_id, expected_origin_uri, nullptr, nullptr, nullptr});
    request->io__ = std::unique_ptr<asapo::IO> {&mock_io};

    auto fname = request->GetFileName();

    ASSERT_THAT(fname, Eq(received));

}


TEST_F(RequestTests, GetStream) {
    strcpy(generic_request_header.stream, expected_stream.c_str());

    request->io__.release();
    request.reset(new Request{generic_request_header, expected_socket_id, expected_origin_uri, nullptr, nullptr, nullptr});
    request->io__ = std::unique_ptr<asapo::IO> {&mock_io};

    auto stream = request->GetStream();

    ASSERT_THAT(stream, Eq(expected_stream));
}


TEST_F(RequestTests, GetFileName) {
    ExpectFileName("filename.txt", "filename.txt");
}


TEST_F(RequestTests, GetFileNameWithLinPath) {
    ExpectFileName("folder1/folder2/filename.txt",
                   std::string("folder1") + asapo::kPathSeparator + std::string("folder2") + asapo::kPathSeparator
                   + "filename.txt");
}

TEST_F(RequestTests, GetFileNameWithWinPath) {
    ExpectFileName("folder1\\folder2\\filename.txt",
                   std::string("folder1") + asapo::kPathSeparator + std::string("folder2") + asapo::kPathSeparator
                   + "filename.txt");
}

TEST_F(RequestTests, SetGetBeamtimeId) {
    request->SetBeamtimeId("beamtime");

    ASSERT_THAT(request->GetBeamtimeId(), "beamtime");
}


TEST_F(RequestTests, SetGetSource) {
    request->SetDataSource("source");

    ASSERT_THAT(request->GetDataSource(), "source");
}


TEST_F(RequestTests, SetGetBeamline) {
    request->SetBeamline("beamline");

    ASSERT_THAT(request->GetBeamline(), "beamline");
}

TEST_F(RequestTests, SetGetSocket) {
    ASSERT_THAT(request->GetSocket(), expected_socket_id);
}

TEST_F(RequestTests, SetGetMetadata) {
    request->SetMetadata("aaa");
    ASSERT_THAT(request->GetMetaData(), "aaa");
}


TEST_F(RequestTests, SetGetFacility) {
    request->SetOnlinePath("p00");
    ASSERT_THAT(request->GetOnlinePath(), "p00");
}

TEST_F(RequestTests, RequestTests_SetGetBeamtimeYear_Test) {
    request->SetOfflinePath("2020");
    ASSERT_THAT(request->GetOfflinePath(), "2020");
}

TEST_F(RequestTests, SetGetWarningMessage) {
    request->SetResponseMessage("warn", asapo::ResponseMessageType::kWarning);

    ASSERT_THAT(request->GetResponseMessage(), "warn");
    ASSERT_THAT(request->GetResponseMessageType(), asapo::ResponseMessageType::kWarning);

}

TEST_F(RequestTests, SetGetInfossage) {
    request->SetResponseMessage("info", asapo::ResponseMessageType::kInfo);

    ASSERT_THAT(request->GetResponseMessage(), "info");
    ASSERT_THAT(request->GetResponseMessageType(), asapo::ResponseMessageType::kInfo);

}

TEST_F(RequestTests, SetGetOverwriteAllowed) {
    request->SetAlreadyProcessedFlag();

    ASSERT_THAT(request->WasAlreadyProcessed(), true);
}


}
