#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "unittests/MockLogger.h"
#include "common/error.h"

#include "../src/request.h"
#include "../src/request_pool.h"
#include "io/io_factory.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::AllOf;


using ::testing::InSequence;
using ::testing::HasSubstr;

using asapo::Request;
using asapo::RequestPool;
using asapo::Error;


TEST(RequestPool, Constructor) {
    asapo::RequestPool pool{4, 4};
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(pool.log__), Ne(nullptr));
}


class MockRequest : public Request {
  public:
    MockRequest() : Request(asapo::GenerateDefaultIO(), asapo::GenericNetworkRequestHeader{}, nullptr, nullptr) {};
    Error Send(asapo::SocketDescriptor* sd, const asapo::ReceiversList& receivers_list) override {
        return Error {Send_t(sd, receivers_list)};
    }

    MOCK_METHOD2(Send_t, asapo::SimpleError*(asapo::SocketDescriptor*, const asapo::ReceiversList&));
};

class RequestPoolTests : public testing::Test {
  public:
    testing::NiceMock<asapo::MockLogger> mock_logger;
    const uint8_t nthreads = 4;
    const uint64_t max_size = 1024 * 1024 * 1024;
    asapo::RequestPool pool {nthreads, max_size};
    std::unique_ptr<Request> request;
    MockRequest* mock_request = new MockRequest;
    void SetUp() override {
        pool.log__ = &mock_logger;
        request.reset(mock_request);
    }
    void TearDown() override {
    }
};



TEST(RequestPool, AddRequestFailsDueToSize) {
    RequestPool pool{4, 0};

    auto  io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    asapo::GenericNetworkRequestHeader header;
    std::unique_ptr<Request> request{new Request{io.get(), header, nullptr, [](asapo::GenericNetworkRequestHeader, asapo::Error) {}}};
    auto err = pool.AddRequest(std::move(request));
    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kRequestPoolIsFull));

}

TEST_F(RequestPoolTests, AddRequestCallsSend) {
    EXPECT_CALL(*mock_request, Send_t(testing::Pointee(asapo::kDisconnectedSocketDescriptor), testing::ElementsAre("test"))).
        WillOnce(
            Return(nullptr)
        );


    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(RequestPoolTests, AddRequestCallsSendTwice) {
    asapo::SimpleError* send_error = new asapo::SimpleError("www");
    EXPECT_CALL(*mock_request, Send_t(testing::Pointee(asapo::kDisconnectedSocketDescriptor), testing::ElementsAre("test")))
        .Times(2)
        .WillOnce(Return(send_error))
        .WillOnce(Return(nullptr));

    auto err = pool.AddRequest(std::move(request));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_THAT(err, Eq(nullptr));


}


TEST_F(RequestPoolTests, FinishProcessingThreads) {
    EXPECT_CALL(mock_logger, Debug(HasSubstr("finishing thread"))).Times(nthreads);
}


/*

void RequestTests::ExpectFailSendHeader(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(expected_file_id,
                                    expected_file_size),
                                    sizeof(asapo::GenericNetworkRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
            HasSubstr("cannot send header"),
            HasSubstr(receivers_list[i])
                                       )
        ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}

void RequestTests::ExpectFailSendData(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, expected_file_pointer, expected_file_size, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
            HasSubstr("cannot send data"),
            HasSubstr(receivers_list[i])
                                       )
        ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}


void RequestTests::ExpectFailReceive(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                testing::Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
            HasSubstr("cannot receive"),
            HasSubstr(receivers_list[i])
                                      )
        ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }

}


void RequestTests::ExpectOKSendData(bool only_once) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, expected_file_pointer, expected_file_size, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(expected_file_size)
            ));
        if (only_once) break;
    }

}



void RequestTests::ExpectOKSendHeader(bool only_once) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(expected_file_id,
                                    expected_file_size),
                                    sizeof(asapo::GenericNetworkRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(sizeof(asapo::GenericNetworkRequestHeader))
            ));
        if (only_once) break;
    }

}


void RequestTests::ExpectOKConnect(bool only_once) {
    int i = 0;
    for (auto expected_address : receivers_list) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(nullptr),
                Return(expected_sds[i])
            ));
        EXPECT_CALL(mock_logger, Info(AllOf(
            HasSubstr("connected"),
            HasSubstr(expected_address)
          )
        ));
        if (only_once) break;
        i++;
    }
}


void RequestTests::ExpectOKReceive(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteSendDataResponse(asapo::kNetErrorNoError),
                testing::ReturnArg<2>()
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
            HasSubstr("sent data"),
            HasSubstr(receivers_list[i])
                                       )
        ));
        if (only_once) break;
        i++;
    }
}


TEST_F(RequestTests, TriesConnectWhenNotConnected) {
    ExpectFailConnect();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestTests, DoesNotTryConnectWhenConnected) {
    sd = expected_sds[0];
    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(_, _))
    .Times(0);
    ExpectFailSendHeader(true);


    auto err = request.Send(&sd, asapo::ReceiversList{expected_address1});

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}



TEST_F(RequestTests, ErrorWhenCannotSendHeader) {
    ExpectOKConnect();
    ExpectFailSendHeader();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestTests, ErrorWhenCannotSendData) {
    ExpectOKConnect();
    ExpectOKSendHeader();
    ExpectFailSendData();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}

TEST_F(RequestTests, ErrorWhenCannotReceiveData) {
    ExpectOKConnect();
    ExpectOKSendHeader();
    ExpectOKSendData();

    ExpectFailReceive();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(asapo::ProducerErrorTemplates::kCannotSendDataToReceivers));
}




TEST_F(RequestTests, ImmediatelyCalBackErrorIfFileAlreadyInUse) {
    ExpectOKConnect(true);
    ExpectOKSendHeader(true);
    ExpectOKSendData(true);

    EXPECT_CALL(mock_io, Receive_t(expected_sds[0], _, sizeof(asapo::SendDataResponse), _))
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(asapo::kNetErrorFileIdAlreadyInUse),
            testing::ReturnArg<2>()
        ));


    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kFileIdAlreadyInUse));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(RequestTests, SendOK) {
    ExpectOKConnect(true);
    ExpectOKSendHeader(true);
    ExpectOKSendData(true);
    ExpectOKReceive();

    auto err = request.Send(&sd, receivers_list);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(sd, Eq(expected_sds[0]));
    ASSERT_THAT(callback_err, Eq(nullptr));
    ASSERT_THAT(called, Eq(true));
    ASSERT_THAT(callback_header.data_size, Eq(header.data_size));
    ASSERT_THAT(callback_header.op_code, Eq(header.op_code));
    ASSERT_THAT(callback_header.data_id, Eq(header.data_id));

}


*/
}
