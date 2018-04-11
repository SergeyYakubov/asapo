#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unittests/MockIO.h>
#include "../src/connection.h"
#include "../src/receiver_error.h"
#include "../src/request.h"
#include "../src/request_handler.h"
#include "../src/request_handler_file_write.h"
#include "mock_statistics.h"

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
using ::hidra2::Error;
using ::hidra2::ErrorInterface;
using ::hidra2::FileDescriptor;
using ::hidra2::SocketDescriptor;
using ::hidra2::GenericNetworkRequestHeader;
using ::hidra2::SendDataResponse;
using ::hidra2::GenericNetworkRequestHeader;
using ::hidra2::GenericNetworkResponse;
using ::hidra2::Opcode;
using ::hidra2::Connection;
using ::hidra2::MockIO;
using hidra2::Request;
using hidra2::MockStatistics;

using hidra2::StatisticEntity;

namespace {

class MockReqestHandler : public hidra2::RequestHandler {
  public:
    Error ProcessRequest(const Request& request) const override {
        return Error{ProcessRequest_t(request)};
    }

    StatisticEntity GetStatisticEntity() const override {
        return StatisticEntity::kDisk;
    }

    MOCK_CONST_METHOD1(ProcessRequest_t, ErrorInterface * (const Request& request));

};

class FactoryTests : public Test {
  public:
    hidra2::RequestFactory factory;
    Error err{nullptr};
    GenericNetworkRequestHeader generic_request_header;
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST_F(FactoryTests, ErrorOnWrongCode) {
    generic_request_header.op_code = hidra2::Opcode::kNetOpcodeUnknownOp;
    auto request = factory.GenerateRequest(generic_request_header, 1, &err);

    ASSERT_THAT(err, Ne(nullptr));
}

TEST_F(FactoryTests, ReturnsDataRequestOnkNetOpcodeSendDataCode) {
    generic_request_header.op_code = hidra2::Opcode::kNetOpcodeSendData;
    auto request = factory.GenerateRequest(generic_request_header, 1, &err);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(dynamic_cast<hidra2::Request*>(request.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const hidra2::RequestHandlerFileWrite*>(request->GetListHandlers()[0]), Ne(nullptr));
}




class RequestTests : public Test {
  public:
    GenericNetworkRequestHeader generic_request_header;
    hidra2::SocketDescriptor socket_fd_{1};
    uint64_t data_size_ {100};
    uint64_t data_id_{15};
    std::unique_ptr<Request> request;
    NiceMock<MockIO> mock_io;
    NiceMock<MockStatistics> mock_statistics;
    std::unique_ptr<hidra2::Statistics>  stat;
    void SetUp() override {
        stat = std::unique_ptr<hidra2::Statistics> {&mock_statistics};
        generic_request_header.data_size = data_size_;
        generic_request_header.data_id = data_id_;
        request.reset(new Request{generic_request_header, socket_fd_});
        request->io__ = std::unique_ptr<hidra2::IO> {&mock_io};
        ON_CALL(mock_io, Receive_t(socket_fd_, _, data_size_, _)).WillByDefault(
            DoAll(SetArgPointee<3>(nullptr),
                  Return(0)
                 ));
    }
    void TearDown() override {
        request->io__.release();
        stat.release();
    }

};

TEST_F(RequestTests, HandleDoesNotReceiveEmptyData) {
    generic_request_header.data_size = 0;
    request->io__.release();
    request.reset(new Request{generic_request_header, socket_fd_});
    request->io__ = std::unique_ptr<hidra2::IO> {&mock_io};;

    EXPECT_CALL(mock_io, Receive_t(_, _, _, _)).Times(0);

    auto err = request->Handle(&stat);

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestTests, HandleReturnsErrorOnDataReceive) {
    EXPECT_CALL(mock_io, Receive_t(socket_fd_, _, data_size_, _)).WillOnce(
        DoAll(SetArgPointee<3>(new hidra2::IOError("Test Read Error", hidra2::IOErrorType::kReadError)),
              Return(0)
             ));

    auto err = request->Handle(&stat);
    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kReadError));
}



TEST_F(RequestTests, HandleMeasuresTimeOnDataReceive) {

    EXPECT_CALL(mock_statistics, StartTimer_t(hidra2::StatisticEntity::kNetwork));

    EXPECT_CALL(mock_io, Receive_t(socket_fd_, _, data_size_, _)).WillOnce(
        DoAll(SetArgPointee<3>(nullptr),
              Return(0)
             ));

    EXPECT_CALL(mock_statistics, StopTimer_t());

    request->Handle(&stat);
}




TEST_F(RequestTests, HandleProcessesRequests) {

    MockReqestHandler mock_request_handler;

    EXPECT_CALL(mock_statistics, StartTimer_t(hidra2::StatisticEntity::kNetwork));

    EXPECT_CALL(mock_request_handler, ProcessRequest_t(_)).WillOnce(
        Return(nullptr)
    ).WillOnce(
        Return(new hidra2::IOError("Test Send Error", hidra2::IOErrorType::kUnknownIOError))
    );;

    request->AddHandler(&mock_request_handler);
    request->AddHandler(&mock_request_handler);

    EXPECT_CALL(mock_statistics, StartTimer_t(hidra2::StatisticEntity::kDisk)).Times(2);

    EXPECT_CALL(mock_statistics, StopTimer_t()).Times(2);


    auto err = request->Handle(&stat);

    ASSERT_THAT(err, Eq(hidra2::IOErrorTemplates::kUnknownIOError));
}

TEST_F(RequestTests, DataIsNullAtInit) {
    auto& data = request->GetData();

    ASSERT_THAT(data, Eq(nullptr));
}

TEST_F(RequestTests, GetDataIsNotNullptr) {

    request->Handle(&stat);
    auto& data = request->GetData();


    ASSERT_THAT(data, Ne(nullptr));


}


TEST_F(RequestTests, GetDataSize) {
    auto size = request->GetDataSize();

    ASSERT_THAT(size, Eq(data_size_));
}

TEST_F(RequestTests, GetFileName) {
    auto fname = request->GetFileName();
    auto s = std::to_string(data_id_) + ".bin";

    ASSERT_THAT(fname, Eq(s));
}


}
