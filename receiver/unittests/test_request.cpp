#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_write.h"
#include "../src/request_handler_db_write.h"
#include "database/database.h"

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
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::GenericRequestHeader;
using ::asapo::SendDataResponse;
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

class RequestTests : public Test {
  public:
    GenericRequestHeader generic_request_header;
    asapo::SocketDescriptor socket_fd_{1};
    uint64_t data_size_ {100};
    uint64_t data_id_{15};
    uint64_t expected_slot_id{16};
    std::string expected_origin_uri = "origin_uri";
    std::string expected_metadata = "meta";
    uint64_t expected_metadata_size = expected_metadata.size();
    asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;
    char expected_request_message[asapo::kMaxMessageSize] = "test_message";
    std::unique_ptr<Request> request;
    NiceMock<MockIO> mock_io;
    NiceMock<MockStatistics> mock_statistics;
    asapo::ReceiverStatistics*  stat;
    MockDataCache mock_cache;
    void SetUp() override {
        stat = &mock_statistics;
        generic_request_header.data_size = data_size_;
        generic_request_header.data_id = data_id_;
        generic_request_header.meta_size = expected_metadata_size;
        generic_request_header.op_code = expected_op_code;
        strcpy(generic_request_header.message, expected_request_message);
        request.reset(new Request{generic_request_header, socket_fd_, expected_origin_uri, nullptr});
        request->io__ = std::unique_ptr<asapo::IO> {&mock_io};
        ON_CALL(mock_io, Receive_t(socket_fd_, _, data_size_, _)).WillByDefault(
            DoAll(SetArgPointee<3>(nullptr),
                  Return(0)
                 ));
    }
    void TearDown() override {
        request->io__.release();
    }
    void ExpectFileName(std::string sended, std::string received);
    void ExpectReceive(uint64_t expected_size, bool ok = true);
    void ExpectReceiveData(bool ok = true);
    void ExpectReceiveMetaData(bool ok = true);
    void ExpectReceiveAllOK();


};

ACTION_P(CopyStr, value) {
    if (value.size() <= arg2 && value.size() > 0) {
        memcpy(static_cast<char*>(arg1), value.c_str(), value.size());
    }
}


void RequestTests::ExpectReceive(uint64_t expected_size, bool ok) {
    EXPECT_CALL(mock_io, Receive_t(socket_fd_, _, expected_size, _)).WillOnce(
        DoAll(
            CopyStr(expected_metadata),
            SetArgPointee<3>(ok ? nullptr : new asapo::IOError("Test Read Error", asapo::IOErrorType::kReadError)),
            Return(0)
        ));

}
void RequestTests::ExpectReceiveData(bool ok) {
    ExpectReceive(data_size_, ok);
}
void RequestTests::ExpectReceiveMetaData(bool ok) {
    ExpectReceive(expected_metadata_size, ok);
}

void RequestTests::ExpectReceiveAllOK() {
    ExpectReceiveData(true);
    ExpectReceiveMetaData(true);
}

TEST_F(RequestTests, HandleDoesNotReceiveEmptyData) {
    generic_request_header.data_size = 0;
    generic_request_header.meta_size = 0;
    request->io__.release();
    request.reset(new Request{generic_request_header, socket_fd_, "", nullptr});
    request->io__ = std::unique_ptr<asapo::IO> {&mock_io};;

    EXPECT_CALL(mock_io, Receive_t(_, _, _, _)).Times(0);

    auto err = request->Handle(stat);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestTests, HandleReturnsErrorOnDataReceive) {
    ExpectReceiveData(false);
    auto err = request->Handle(stat);
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kReadError));
}

TEST_F(RequestTests, HandleReturnsErrorOnMetaDataReceive) {
    ExpectReceiveData(true);
    ExpectReceiveMetaData(false);
    auto err = request->Handle(stat);
    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kReadError));
}



TEST_F(RequestTests, HandleGetsMemoryFromCache) {
    request->cache__ = &mock_cache;
    asapo::CacheMeta meta;
    meta.id = expected_slot_id;
    EXPECT_CALL(mock_cache, GetFreeSlotAndLock(data_size_, _)).WillOnce(
        DoAll(SetArgPointee<1>(&meta),
              Return(&mock_cache)
             ));

    EXPECT_CALL(mock_cache, UnlockSlot(&meta));

    request->Handle(stat);

    ASSERT_THAT(request->GetSlotId(), Eq(expected_slot_id));
}


TEST_F(RequestTests, ErrorGetMemoryFromCache) {
    request->cache__ = &mock_cache;

    EXPECT_CALL(mock_cache, GetFreeSlotAndLock(data_size_, _)).WillOnce(
        Return(nullptr)
    );

    EXPECT_CALL(mock_cache, UnlockSlot(_)).Times(0);


    auto err = request->Handle(stat);

    ASSERT_THAT(request->GetSlotId(), Eq(0));
    ASSERT_THAT(err, Eq(asapo::ErrorTemplates::kMemoryAllocationError));
}


TEST_F(RequestTests, HandleMeasuresTimeOnContentReceive) {

    EXPECT_CALL(mock_statistics, StartTimer_t(asapo::StatisticEntity::kNetwork));

    ExpectReceiveAllOK();

    EXPECT_CALL(mock_statistics, StopTimer_t());

    request->Handle(stat);


    ASSERT_THAT(request->GetMetaData(), Eq(expected_metadata));


}


TEST_F(RequestTests, HandleProcessesRequests) {

    MockReqestHandler mock_request_handler;

    EXPECT_CALL(mock_statistics, StartTimer_t(asapo::StatisticEntity::kNetwork));

    EXPECT_CALL(mock_request_handler, ProcessRequest_t(_)).WillOnce(
        Return(nullptr)
    ).WillOnce(
        Return(new asapo::IOError("Test Send Error", asapo::IOErrorType::kUnknownIOError))
    );;

    request->AddHandler(&mock_request_handler);
    request->AddHandler(&mock_request_handler);

    EXPECT_CALL(mock_statistics, StartTimer_t(asapo::StatisticEntity::kDisk)).Times(2);

    EXPECT_CALL(mock_statistics, StopTimer_t()).Times(2);


    auto err = request->Handle(stat);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}

TEST_F(RequestTests, DataIsNullAtInit) {
    auto data = request->GetData();
    ASSERT_THAT(data, Eq(nullptr));
}

TEST_F(RequestTests, GetDataIsNotNullptr) {

    request->Handle(stat);
    auto data = request->GetData();


    ASSERT_THAT(data, Ne(nullptr));


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


TEST_F(RequestTests, OriginUriEmptyByDefault) {
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
    request.reset(new Request{generic_request_header, socket_fd_, expected_origin_uri, nullptr});
    request->io__ = std::unique_ptr<asapo::IO> {&mock_io};;

    auto fname = request->GetFileName();

    ASSERT_THAT(fname, Eq(received));

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


TEST_F(RequestTests, SetGetStream) {
    request->SetStream("stream");

    ASSERT_THAT(request->GetStream(), "stream");
}


TEST_F(RequestTests, SetGetBeamline) {
    request->SetBeamline("beamline");

    ASSERT_THAT(request->GetBeamline(), "beamline");
}



}
