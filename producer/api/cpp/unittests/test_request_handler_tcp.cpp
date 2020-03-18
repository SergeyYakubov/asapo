#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "unittests/MockIO.h"
#include "unittests/MockLogger.h"
#include "common/error.h"
#include "io/io.h"

#include "producer/common.h"
#include "producer/producer_error.h"

#include "../src/request_handler_tcp.h"
#include <common/networking.h>
#include "io/io_factory.h"

#include "mocking.h"

#include <functional>

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
using testing::NiceMock;

using ::testing::InSequence;
using ::testing::HasSubstr;
using ::testing::Sequence;


TEST(RequestHandlerTcp, Constructor) {
    MockDiscoveryService ds;
    asapo::RequestHandlerTcp request{&ds, 1, nullptr};

    ASSERT_THAT(dynamic_cast<const asapo::IO*>(request.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(request.log__), Ne(nullptr));
    ASSERT_THAT(request.discovery_service__, Eq(&ds));

}
std::string expected_auth_message = {"12345"};
asapo::Opcode expected_op_code = asapo::kOpcodeTransferData;

class RequestHandlerTcpTests : public testing::Test {
  public:
    NiceMock<asapo::MockIO> mock_io;
    NiceMock<MockDiscoveryService> mock_discovery_service;

    uint64_t expected_file_id = 42;
    uint64_t expected_file_size = 1337;
    uint64_t expected_meta_size = 4;
    std::string expected_metadata = "meta";
    std::string expected_warning = "warning";
    char  expected_file_name[asapo::kMaxMessageSize] = "test_name";
    char  expected_beamtime_id[asapo::kMaxMessageSize] = "test_beamtime_id";
    char  expected_substream[asapo::kMaxMessageSize] = "test_substream";

    uint64_t expected_thread_id = 2;

    asapo::Error callback_err;
    asapo::GenericRequestHeader header{expected_op_code, expected_file_id, expected_file_size,
              expected_meta_size, expected_file_name, expected_substream};
    asapo::GenericRequestHeader header_fromfile{expected_op_code, expected_file_id, 0, expected_meta_size,
              expected_file_name, expected_substream};
    bool callback_called = false;
    asapo::GenericRequestHeader callback_header;


    asapo::ProducerRequest request{expected_beamtime_id, header, nullptr, expected_metadata, "", [this](asapo::GenericRequestHeader header, asapo::Error err) {
            callback_called = true;
            callback_err = std::move(err);
            callback_header = header;
        }, true, 0};

    std::string expected_origin_fullpath = std::string("origin/") + expected_file_name;
    asapo::ProducerRequest request_filesend{expected_beamtime_id, header_fromfile, nullptr, expected_metadata,
        expected_origin_fullpath, [this](asapo::GenericRequestHeader header, asapo::Error err) {
            callback_called = true;
            callback_err = std::move(err);
            callback_header = header;
        }, true, 0};


    asapo::ProducerRequest request_nocallback{expected_beamtime_id, header, nullptr, expected_metadata,  "", nullptr, true, 0};
    testing::NiceMock<asapo::MockLogger> mock_logger;
    uint64_t n_connections{0};
    asapo::RequestHandlerTcp request_handler{&mock_discovery_service, expected_thread_id, &n_connections};

    std::string expected_address1 = {"127.0.0.1:9090"};
    std::string expected_address2 = {"127.0.0.1:9091"};
    asapo::ReceiversList receivers_list{expected_address1, expected_address2};
    asapo::ReceiversList receivers_list2{expected_address2, expected_address1};

    asapo::ReceiversList receivers_list_single{expected_address1};

    std::vector<asapo::SocketDescriptor> expected_sds{83942, 83943};

    bool retry;
    Sequence seq_receive[2];
    void ExpectFailConnect(bool only_once = false);
    void ExpectFailAuthorize(bool only_once = false);
    void ExpectOKAuthorize(bool only_once = false);
    void ExpectFailSendHeader(bool only_once = false);
    void ExpectFailSend(uint64_t expected_size, bool only_once);
    void ExpectFailSendData(bool only_once = false);
    void ExpectFailSendMetaData(bool only_once = false);
    void ExpectOKConnect(bool only_once = false);
    void ExpectOKSendHeader(bool only_once = false, asapo::Opcode code = expected_op_code);
    void ExpectOKSend(uint64_t expected_size, bool only_once);
    void ExpectOKSendAll(bool only_once);
    void ExpectGetFileSize(bool ok);
    void ExpectOKSendData(bool only_once = false);
    void ExpectOKSendFile(bool only_once = false);
    void ExpectFailSendFile(const asapo::ProducerErrorTemplate& err_template, bool client_error = false);
    void ExpectOKSendMetaData(bool only_once = false);
    void ExpectFailReceive(bool only_once = false);
    void ExpectOKReceive(bool only_once = true, asapo::NetworkErrorCode code = asapo::kNetErrorNoError,
                         std::string message = "");
    void DoSingleSend(bool connect = true, bool success = true);
    void AssertImmediatelyCallBack(asapo::NetworkErrorCode error_code, const asapo::ProducerErrorTemplate& err_template);
    void SetUp() override {
        request_handler.log__ = &mock_logger;
        request_handler.io__.reset(&mock_io);
        request.header.custom_data[asapo::kPosIngestMode] = asapo::kDefaultIngestMode;
        request_filesend.header.custom_data[asapo::kPosIngestMode] = asapo::kDefaultIngestMode;
        request_nocallback.header.custom_data[asapo::kPosIngestMode] = asapo::kDefaultIngestMode;
        ON_CALL(mock_discovery_service, RotatedUriList(_)).
        WillByDefault(Return(receivers_list));

    }
    void TearDown() override {
        request_handler.io__.release();
    }
};

ACTION_P2(A_WriteSendDataResponse, error_code, message) {
    ((asapo::SendDataResponse*)arg1)->op_code = asapo::kOpcodeTransferData;
    ((asapo::SendDataResponse*)arg1)->error_code = error_code;
    strcpy(((asapo::SendDataResponse*)arg1)->message, message.c_str());
}

MATCHER_P5(M_CheckSendDataRequest, op_code, file_id, file_size, message, substream,
           "Checks if a valid GenericRequestHeader was Send") {
    return ((asapo::GenericRequestHeader*)arg)->op_code == op_code
           && ((asapo::GenericRequestHeader*)arg)->data_id == uint64_t(file_id)
           && ((asapo::GenericRequestHeader*)arg)->data_size == uint64_t(file_size)
           && strcmp(((asapo::GenericRequestHeader*)arg)->message, message) == 0
           && strcmp(((asapo::GenericRequestHeader*)arg)->substream, substream) == 0;

}


void RequestHandlerTcpTests::ExpectFailConnect(bool only_once) {
    for (auto expected_address : receivers_list) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(asapo::IOErrorTemplates::kInvalidAddressFormat.Generate().release()),
                Return(asapo::kDisconnectedSocketDescriptor)
            ));
        if (only_once)
            EXPECT_CALL(mock_logger, Debug(AllOf(
                                               HasSubstr("cannot connect"),
                                               HasSubstr(expected_address)
                                           )
                                          ));

        if (only_once) break;
    }

}


void RequestHandlerTcpTests::ExpectFailAuthorize(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(asapo::kOpcodeAuthorize, 0, 0, expected_beamtime_id,
                                    ""),
                                    sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(sizeof(asapo::GenericRequestHeader))
            ));

        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .InSequence(seq_receive[i])
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteSendDataResponse(asapo::kNetAuthorizationError, expected_auth_message),
                testing::ReturnArg<2>()
            ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) {
            EXPECT_CALL(mock_logger, Debug(AllOf(
                                               HasSubstr("disconnected"),
                                               HasSubstr(receivers_list[i])
                                           )
                                          ));

            EXPECT_CALL(mock_logger, Error(AllOf(
                                               HasSubstr("authorization"),
                                               HasSubstr(expected_auth_message),
                                               HasSubstr(receivers_list[i])
                                           )
                                          ));
        }
        if (only_once) break;
        i++;
    }
}

void RequestHandlerTcpTests::ExpectOKAuthorize(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(asapo::kOpcodeAuthorize, 0, 0, expected_beamtime_id,
                                    ""),
                                    sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(sizeof(asapo::GenericRequestHeader))
            ));


        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .InSequence(seq_receive[i])
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteSendDataResponse(asapo::kNetErrorNoError, expected_auth_message),
                testing::ReturnArg<2>()
            ));
        if (only_once) {
            EXPECT_CALL(mock_logger, Info(AllOf(
                                              HasSubstr("authorized"),
                                              HasSubstr(receivers_list[i])
                                          )
                                         ));
        }
        if (only_once) break;
        i++;
    }

}



void RequestHandlerTcpTests::ExpectFailSendHeader(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(expected_op_code, expected_file_id,
                                    expected_file_size, expected_file_name, expected_substream),
                                    sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(-1)
            ));
        if (only_once) {
            EXPECT_CALL(mock_logger, Debug(AllOf(
                                               HasSubstr("disconnected"),
                                               HasSubstr(receivers_list[i])
                                           )
                                          ));

            EXPECT_CALL(mock_logger, Warning(AllOf(
                                                 HasSubstr("cannot send"),
                                                 HasSubstr(receivers_list[i])
                                             )
                                            ));
        }
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }
    if (only_once) {
        EXPECT_CALL(mock_logger, Warning(HasSubstr("put back")));
    }
}

void RequestHandlerTcpTests::ExpectFailSendFile(const asapo::ProducerErrorTemplate& err_template, bool client_error) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, SendFile_t(expected_sd, expected_origin_fullpath, (size_t) expected_file_size))
        .Times(1)
        .WillOnce(
            Return(err_template.Generate().release())
        );

        if (client_error) {

            EXPECT_CALL(mock_logger, Debug(AllOf(
                                               HasSubstr("disconnected"),
                                               HasSubstr(receivers_list[i])
                                           )
                                          ));
            EXPECT_CALL(mock_logger, Error(AllOf(
                                               HasSubstr("cannot send"),
                                               HasSubstr(receivers_list[i])             )
                                          ));

        }

        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (client_error) break;
        i++;
    }
    if (client_error && err_template != asapo::ProducerErrorTemplates::kLocalIOError.Generate()) {
        EXPECT_CALL(mock_logger, Warning(HasSubstr("put back")));
    }

}


void RequestHandlerTcpTests::ExpectFailSend(uint64_t expected_size, bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, _, (size_t) expected_size, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                Return(-1)
            ));
        if (only_once) {
            EXPECT_CALL(mock_logger, Debug(AllOf(
                                               HasSubstr("disconnected"),
                                               HasSubstr(receivers_list[i])
                                           )
                                          ));

            EXPECT_CALL(mock_logger, Warning(AllOf(
                                                 HasSubstr("cannot send"),
                                                 HasSubstr(receivers_list[i])
                                             )
                                            ));

        }
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }
    if (only_once) EXPECT_CALL(mock_logger, Warning(HasSubstr("put back")));
}

void RequestHandlerTcpTests::ExpectFailSendData(bool only_once) {
    ExpectFailSend(expected_file_size, only_once);
}

void RequestHandlerTcpTests::ExpectFailSendMetaData(bool only_once) {
    ExpectFailSend(expected_meta_size, only_once);
}



void RequestHandlerTcpTests::ExpectFailReceive(bool only_once) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .InSequence(seq_receive[i])
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(asapo::IOErrorTemplates::kBadFileNumber.Generate().release()),
                testing::Return(-1)
            ));
        EXPECT_CALL(mock_logger, Debug(AllOf(
                                           HasSubstr("disconnected"),
                                           HasSubstr(receivers_list[i])
                                       )
                                      ));


        EXPECT_CALL(mock_logger, Warning(AllOf(
                                             HasSubstr("cannot send"),
                                             HasSubstr(receivers_list[i])
                                         )
                                        ));
        EXPECT_CALL(mock_io, CloseSocket_t(expected_sd, _));
        if (only_once) break;
        i++;
    }
    EXPECT_CALL(mock_logger, Warning(HasSubstr("put back")));

}

void RequestHandlerTcpTests::ExpectOKSendAll(bool only_once) {
    ExpectOKSendHeader(only_once);
    ExpectOKSendMetaData(only_once);
    ExpectOKSendData(only_once);
}


void RequestHandlerTcpTests::ExpectOKSend(uint64_t expected_size, bool only_once) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, _, (size_t)expected_size, _))
        .Times(1)
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return((size_t) expected_file_size)
            ));
        if (only_once) break;
    }
}

void RequestHandlerTcpTests::ExpectOKSendMetaData(bool only_once) {
    ExpectOKSend(expected_meta_size, only_once);
}



void RequestHandlerTcpTests::ExpectOKSendData(bool only_once) {
    ExpectOKSend(expected_file_size, only_once);
}

void RequestHandlerTcpTests::ExpectOKSendFile(bool only_once) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, SendFile_t(expected_sd, expected_origin_fullpath, (size_t)expected_file_size))
        .Times(1)
        .WillOnce(Return(nullptr));
        if (only_once) break;
    }
}

void RequestHandlerTcpTests::ExpectOKSendHeader(bool only_once, asapo::Opcode opcode) {
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Send_t(expected_sd, M_CheckSendDataRequest(opcode, expected_file_id,
                                    expected_file_size, expected_file_name, expected_substream),
                                    sizeof(asapo::GenericRequestHeader), _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                Return(sizeof(asapo::GenericRequestHeader))
            ));
        if (only_once) break;
    }

}


void RequestHandlerTcpTests::ExpectOKConnect(bool only_once) {
    int i = 0;
    for (auto expected_address : receivers_list) {
        EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(expected_address, _))
        .WillOnce(
            DoAll(
                testing::SetArgPointee<1>(nullptr),
                Return(expected_sds[i])
            ));
        if (only_once) {
            EXPECT_CALL(mock_logger, Debug(AllOf(
                                               HasSubstr("connected to"),
                                               HasSubstr(expected_address)
                                           )
                                          ));
        }
        if (only_once) break;
        i++;
    }
}


void RequestHandlerTcpTests::ExpectOKReceive(bool only_once, asapo::NetworkErrorCode code, std::string message) {
    int i = 0;
    for (auto expected_sd : expected_sds) {
        EXPECT_CALL(mock_io, Receive_t(expected_sd, _, sizeof(asapo::SendDataResponse), _))
        .InSequence(seq_receive[i])
        .WillOnce(
            DoAll(
                testing::SetArgPointee<3>(nullptr),
                A_WriteSendDataResponse(code, message),
                testing::ReturnArg<2>()
            ));
        if (only_once) {
            EXPECT_CALL(mock_logger, Debug(AllOf(
                                               HasSubstr("sent data"),
                                               HasSubstr(receivers_list[i])
                                           )
                                          ));
        }
        if (only_once) break;
        i++;
    }
}

void RequestHandlerTcpTests::DoSingleSend(bool connect, bool success) {
    if (connect)  {
        ExpectOKConnect(true);
        ExpectOKAuthorize(true);
    }

    ExpectOKSendAll(true);
    if (success) {
        ExpectOKReceive(true);
    } else {
        ExpectFailReceive(true);
    }

    if (connect) {
        EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).
        WillOnce(Return(receivers_list_single));
    }

    request_handler.PrepareProcessingRequestLocked();
    request_handler.ProcessRequestUnlocked(&request, &retry);

    Mock::VerifyAndClearExpectations(&mock_io);
    Mock::VerifyAndClearExpectations(&mock_logger);
    Mock::VerifyAndClearExpectations(&mock_discovery_service);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST_F(RequestHandlerTcpTests, CannotProcessRequestIfNotEnoughConnections) {
    EXPECT_CALL(mock_discovery_service, MaxConnections()).WillOnce(Return(0));
    auto res = request_handler.ReadyProcessRequest();
    ASSERT_THAT(res, Eq(false));
}

TEST_F(RequestHandlerTcpTests, CanProcessRequestIfAlreadyConnected) {
    DoSingleSend();
    EXPECT_CALL(mock_discovery_service, MaxConnections()).Times(0);

    auto res = request_handler.ReadyProcessRequest();

    ASSERT_THAT(res, Eq(true));
}

TEST_F(RequestHandlerTcpTests, GetsUriListINotConnected) {
    EXPECT_CALL(mock_discovery_service, RotatedUriList(_));
    request_handler.PrepareProcessingRequestLocked();
}

TEST_F(RequestHandlerTcpTests, DoesNotGetsUriIfAlreadyConnected) {
    DoSingleSend();
    EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).Times(0);
    request_handler.PrepareProcessingRequestLocked();
}

TEST_F(RequestHandlerTcpTests, ReduceConnectionNumberAtTearDownIfError) {
    n_connections = 1;
    request_handler.TearDownProcessingRequestLocked(false);

    ASSERT_THAT(n_connections, Eq(0));
}

TEST_F(RequestHandlerTcpTests, DoNotReduceConnectionNumberAtTearDownIfNoError) {
    n_connections = 1;

    request_handler.TearDownProcessingRequestLocked(true);

    ASSERT_THAT(n_connections, Eq(1));
}


TEST_F(RequestHandlerTcpTests, TriesConnectWhenNotConnected) {
    ExpectFailConnect();

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));
}

TEST_F(RequestHandlerTcpTests, FailsWhenCannotAuthorize) {
    ExpectOKConnect(true);
    ExpectFailAuthorize(true);

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);
    request_handler.TearDownProcessingRequestLocked(success);

    ASSERT_THAT(n_connections, Eq(0));
    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kWrongInput));
    ASSERT_THAT(callback_called, Eq(true));
    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(false));


}


TEST_F(RequestHandlerTcpTests, DoesNotTryConnectWhenConnected) {
    DoSingleSend();

    EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).
    WillOnce(Return(receivers_list_single));


    EXPECT_CALL(mock_io, CreateAndConnectIPTCPSocket_t(_, _))
    .Times(0);

    ExpectFailSendHeader(true);

    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));

}


TEST_F(RequestHandlerTcpTests, DoNotCloseWhenNotConnected) {
    EXPECT_CALL(mock_io, CloseSocket_t(_, _)).Times(0);
    ExpectOKConnect();
    ExpectOKAuthorize();
    ExpectFailSendHeader();

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));

}


TEST_F(RequestHandlerTcpTests, CloseConnectionWhenRebalance) {
    DoSingleSend();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).
    WillOnce(Return(asapo::ReceiversList{}));

    EXPECT_CALL(mock_io, CloseSocket_t(_, _));

    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));

}



TEST_F(RequestHandlerTcpTests, ErrorWhenCannotSendHeader) {
    ExpectOKConnect();
    ExpectOKAuthorize();
    ExpectFailSendHeader();

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));

}


TEST_F(RequestHandlerTcpTests, ErrorWhenCannotSendData) {
    ExpectOKConnect();
    ExpectOKAuthorize();
    ExpectOKSendHeader();
    ExpectOKSendMetaData();
    ExpectFailSendData();

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));

}

TEST_F(RequestHandlerTcpTests, ErrorWhenCannotSendMetaData) {
    ExpectOKConnect();
    ExpectOKAuthorize();
    ExpectOKSendHeader();
    ExpectFailSendMetaData();

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));

}

TEST_F(RequestHandlerTcpTests, ErrorWhenCannotReceiveData) {
    EXPECT_CALL(mock_discovery_service, RotatedUriList(_)).
    WillOnce(Return(receivers_list_single));

    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendAll(true);
    ExpectFailReceive(true);

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(true));

}

void RequestHandlerTcpTests::AssertImmediatelyCallBack(asapo::NetworkErrorCode error_code,
        const asapo::ProducerErrorTemplate& err_template) {
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendAll(true);

    EXPECT_CALL(mock_io, Receive_t(expected_sds[0], _, sizeof(asapo::SendDataResponse), _))
    .InSequence(seq_receive[0])
    .WillOnce(
        DoAll(
            testing::SetArgPointee<3>(nullptr),
            A_WriteSendDataResponse(error_code, expected_auth_message),
            testing::ReturnArg<2>()
        ));
    EXPECT_CALL(mock_logger, Debug(AllOf(
                                       HasSubstr("disconnected"),
                                       HasSubstr(receivers_list[0])
                                   )
                                  ));

    EXPECT_CALL(mock_logger, Error(AllOf(
                                       HasSubstr("cannot send"),
                                       HasSubstr(receivers_list[0])
                                   )
                                  ));

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);
    ASSERT_THAT(callback_err, Eq(err_template));
    ASSERT_THAT(callback_called, Eq(true));
    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(retry, Eq(false));

}

void RequestHandlerTcpTests::ExpectGetFileSize(bool ok) {
    asapo::FileInfo fi;
    if (ok) {
        fi.size = expected_file_size;
    }

    EXPECT_CALL(mock_io, GetFileInfo_t(expected_origin_fullpath, _)).WillOnce(
        DoAll(
            testing::SetArgPointee<1>(ok ? nullptr : asapo::IOErrorTemplates::kFileNotFound.Generate().release()),
            testing::Return(fi)
        ));
}

TEST_F(RequestHandlerTcpTests, ImmediatelyCallBackErrorIfAuthorizationFailure) {
    AssertImmediatelyCallBack(asapo::kNetAuthorizationError, asapo::ProducerErrorTemplates::kWrongInput);
}


TEST_F(RequestHandlerTcpTests, ImmediatelyCallBackErrorIfWrongMetadata) {
    AssertImmediatelyCallBack(asapo::kNetErrorWrongRequest, asapo::ProducerErrorTemplates::kWrongInput);
}


TEST_F(RequestHandlerTcpTests, SendEmptyCallBack) {
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendAll(true);
    ExpectOKReceive();

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request_nocallback, &retry);

    ASSERT_THAT(success, Eq(true));
    ASSERT_THAT(callback_called, Eq(false));
    ASSERT_THAT(retry, Eq(false));
}

TEST_F(RequestHandlerTcpTests, ErrorWhenCannotSendFileWithReadError) {
    ExpectGetFileSize(true);
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendHeader(true);
    ExpectOKSendMetaData(true);
    ExpectFailSendFile(asapo::ProducerErrorTemplates::kLocalIOError, true);

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request_filesend, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(callback_called, Eq(true));
    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kLocalIOError));
    ASSERT_THAT(retry, Eq(false));

}

TEST_F(RequestHandlerTcpTests, ErrorWhenCannotSendFileWithServerError) {
    ExpectGetFileSize(true);
    ExpectOKConnect();
    ExpectOKAuthorize();
    ExpectOKSendHeader();
    ExpectOKSendMetaData();
    ExpectFailSendFile(asapo::ProducerErrorTemplates::kInternalServerError);

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request_filesend, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(callback_called, Eq(false));
    ASSERT_THAT(retry, Eq(true));
}



TEST_F(RequestHandlerTcpTests, RetryOnReauthorize) {
    ExpectOKConnect(false);
    ExpectOKAuthorize(false);
    ExpectOKSendAll(false);
    ExpectOKReceive(false, asapo::kNetErrorReauthorize);

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(callback_called, Eq(false));
    ASSERT_THAT(retry, Eq(true));
}



TEST_F(RequestHandlerTcpTests, FileRequestErrorGettingFileSize) {
    ExpectGetFileSize(false);

    request_handler.PrepareProcessingRequestLocked();

    auto success = request_handler.ProcessRequestUnlocked(&request_filesend, &retry);
    ASSERT_THAT(success, Eq(false));
    ASSERT_THAT(callback_called, Eq(true));
    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kLocalIOError));
    ASSERT_THAT(retry, Eq(false));

}



TEST_F(RequestHandlerTcpTests, FileRequestOK) {
    ExpectGetFileSize(true);
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendHeader(true);
    ExpectOKSendMetaData(true);
    ExpectOKSendFile(true);
    ExpectOKReceive();

    request_handler.PrepareProcessingRequestLocked();

    auto success = request_handler.ProcessRequestUnlocked(&request_filesend, &retry);
    ASSERT_THAT(success, Eq(true));
    ASSERT_THAT(callback_called, Eq(true));
    ASSERT_THAT(callback_err, Eq(nullptr));
    ASSERT_THAT(retry, Eq(false));

}



TEST_F(RequestHandlerTcpTests, SendOK) {
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendAll(true);
    ExpectOKReceive();

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
    ASSERT_THAT(retry, Eq(false));
    ASSERT_THAT(callback_err, Eq(nullptr));
    ASSERT_THAT(callback_called, Eq(true));
    ASSERT_THAT(callback_header.data_size, Eq(header.data_size));
    ASSERT_THAT(callback_header.op_code, Eq(header.op_code));
    ASSERT_THAT(callback_header.data_id, Eq(header.data_id));
    ASSERT_THAT(std::string{callback_header.message}, Eq(std::string{header.message}));
}

TEST_F(RequestHandlerTcpTests, SendMetadataIgnoresIngestMode) {
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendHeader(true, asapo::kOpcodeTransferMetaData);
    ExpectOKSendData(true);
    ExpectOKSendMetaData(true);
    ExpectOKReceive();

    auto ingest_mode = asapo::IngestModeFlags::kTransferMetaDataOnly;
    request.header.custom_data[asapo::kPosIngestMode] = ingest_mode;
    request.header.op_code = asapo::kOpcodeTransferMetaData;

    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
    ASSERT_THAT(retry, Eq(false));

}


TEST_F(RequestHandlerTcpTests, SendMetaOnlyOK) {
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendHeader(true);
    ExpectOKSendMetaData(true);
    ExpectOKReceive();

    auto ingest_mode = asapo::IngestModeFlags::kTransferMetaDataOnly;

    request.header.custom_data[asapo::kPosIngestMode] = ingest_mode;
    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
    ASSERT_THAT(retry, Eq(false));
    ASSERT_THAT(callback_header.custom_data[asapo::kPosIngestMode], Eq(ingest_mode));
}

TEST_F(RequestHandlerTcpTests, SendMetaOnlyForFileReadOK) {
    expected_file_size = 0;
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendHeader(true);
    ExpectOKSendMetaData(true);
    ExpectOKReceive();

    request_handler.PrepareProcessingRequestLocked();

    EXPECT_CALL(mock_io, SendFile_t(_, _, _)).Times(0);
    EXPECT_CALL(mock_io, GetFileInfo_t(_, _)).Times(0);
    auto ingest_mode = asapo::IngestModeFlags::kTransferMetaDataOnly;

    request_filesend.header.custom_data[asapo::kPosIngestMode] = ingest_mode;
    auto success = request_handler.ProcessRequestUnlocked(&request_filesend, &retry);
    ASSERT_THAT(success, Eq(true));
    ASSERT_THAT(retry, Eq(false));

}


TEST_F(RequestHandlerTcpTests, TimeoutCallsCallback) {
    EXPECT_CALL(mock_logger, Error(AllOf(
                                       HasSubstr("timeout"),
                                       HasSubstr("substream"))
                                  ));

    request_handler.ProcessRequestTimeout(&request);

    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kTimeout));
    ASSERT_THAT(callback_called, Eq(true));
}



TEST_F(RequestHandlerTcpTests, SendWithWarning) {
    ExpectOKConnect(true);
    ExpectOKAuthorize(true);
    ExpectOKSendAll(true);
    ExpectOKReceive(true, asapo::kNetErrorWarning, expected_warning);

    EXPECT_CALL(mock_logger, Warning(AllOf(
                                         HasSubstr("server"),
                                         HasSubstr(expected_warning))
                                    ));


    request_handler.PrepareProcessingRequestLocked();
    auto success = request_handler.ProcessRequestUnlocked(&request, &retry);

    ASSERT_THAT(success, Eq(true));
    ASSERT_THAT(retry, Eq(false));
    ASSERT_THAT(callback_err, Eq(asapo::ProducerErrorTemplates::kServerWarning));
    ASSERT_THAT(callback_err->Explain(), HasSubstr(expected_warning));
    ASSERT_THAT(callback_called, Eq(true));
    ASSERT_THAT(callback_header.data_size, Eq(header.data_size));
    ASSERT_THAT(callback_header.op_code, Eq(header.op_code));
    ASSERT_THAT(callback_header.data_id, Eq(header.data_id));
    ASSERT_THAT(std::string{callback_header.message}, Eq(std::string{header.message}));
}

}
