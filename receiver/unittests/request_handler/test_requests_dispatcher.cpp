#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockLogger.h"
#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/statistics/receiver_statistics.h"
#include "../receiver_mocking.h"
#include "../mock_receiver_config.h"

#include "../../src/request_handler/requests_dispatcher.h"
#include "asapo/database/db_error.h"


using namespace testing;
using namespace asapo;

namespace {

TEST(RequestDispatcher, Constructor) {
    auto stat = std::unique_ptr<ReceiverStatistics> {new ReceiverStatistics};
    RequestsDispatcher dispatcher{0,  "some_address", stat.get(), nullptr};
    ASSERT_THAT(dynamic_cast<const asapo::ReceiverStatistics*>(dispatcher.statistics__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::IO*>(dispatcher.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestFactory*>(dispatcher.request_factory__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(dispatcher.log__), Ne(nullptr));
}

class MockRequest: public Request {
  public:
    MockRequest(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string uri = ""):
        Request(request_header, socket_fd, uri, nullptr, nullptr) {};
    Error Handle(ReceiverStatistics*) override {
        return Error{Handle_t()};
    };
    MOCK_CONST_METHOD0(Handle_t, ErrorInterface * ());
};


class MockRequestFactory: public asapo::RequestFactory {
  public:
    MockRequestFactory(): RequestFactory(nullptr) {};
    std::unique_ptr<Request> GenerateRequest(const GenericRequestHeader& request_header,
                                             SocketDescriptor socket_fd, std::string origin_uri,
                                             Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto res = GenerateRequest_t(request_header, socket_fd, origin_uri, &error);
        err->reset(error);
        return std::unique_ptr<Request> {res};
    }

    MOCK_CONST_METHOD4(GenerateRequest_t, Request * (const GenericRequestHeader&,
                                                     SocketDescriptor, std::string,
                                                     ErrorInterface**));

};


ACTION_P(SaveArg1ToGenericNetworkResponse, value) {
    auto resp =  *static_cast<const GenericNetworkResponse*>(arg1);
    value->error_code = resp.error_code;
    strcpy(value->message, resp.message);
}

class RequestsDispatcherTests : public Test {
  public:
    std::unique_ptr<RequestsDispatcher> dispatcher;
    std::string connected_uri{"some_address"};
    NiceMock<MockIO> mock_io;
    MockRequestFactory mock_factory;
    NiceMock<MockStatistics> mock_statictics;
    NiceMock<asapo::MockLogger> mock_logger;

    asapo::ReceiverConfig test_config;
    GenericRequestHeader header;
    MockRequest mock_request{GenericRequestHeader{}, 1, connected_uri};
    std::unique_ptr<Request> request{&mock_request};
    GenericNetworkResponse response;
    void SetUp() override {
        test_config.authorization_interval_ms = 0;
        SetReceiverConfig(test_config, "none");
        dispatcher = std::unique_ptr<RequestsDispatcher> {new RequestsDispatcher{0, connected_uri, &mock_statictics, nullptr}};
        dispatcher->io__ = std::unique_ptr<asapo::IO> {&mock_io};
        dispatcher->statistics__ = &mock_statictics;
        dispatcher->request_factory__ = std::unique_ptr<asapo::RequestFactory> {&mock_factory};
        dispatcher->log__ = &mock_logger;

    }
    void TearDown() override {
        dispatcher->io__.release();
        dispatcher->request_factory__.release();
        request.release();
    }
    void MockReceiveRequest(bool error ) {
        EXPECT_CALL(mock_io, Receive_t(_, _, _, _))
        .WillOnce(
            DoAll(SetArgPointee<3>(error ? asapo::IOErrorTemplates::kUnknownIOError.Generate().release() : nullptr),
                  Return(0))
        );
        if (error) {
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("getting next request"), HasSubstr(connected_uri))));
        }

    }
    void MockCreateRequest(bool error ) {
        EXPECT_CALL(mock_factory, GenerateRequest_t(_, _, _, _))
        .WillOnce(
            DoAll(SetArgPointee<3>(error ? asapo::ReceiverErrorTemplates::kInvalidOpCode.Generate().release() : nullptr),
                  Return(nullptr))
        );
        if (error) {
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("error processing request"), HasSubstr(connected_uri))));
        }


    }
    void MockHandleRequest(int error_mode, Error err = asapo::IOErrorTemplates::kUnknownIOError.Generate() ) {
        EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("got new request"), HasSubstr(connected_uri))));

        EXPECT_CALL(mock_request, Handle_t()).WillOnce(
            Return(error_mode > 0 ? err.release() : nullptr)
        );
        if (error_mode == 1) {
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("error processing request"), HasSubstr(connected_uri))));
        } else if (error_mode == 2) {
            EXPECT_CALL(mock_logger, Warning(AllOf(HasSubstr("warning processing request"), HasSubstr(connected_uri))));
        }
    }
    void MockSendResponse(GenericNetworkResponse* response, bool error ) {
        EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("sending response"), HasSubstr(connected_uri))));
        EXPECT_CALL(mock_io, Send_t(_, _, _, _)).WillOnce(
            DoAll(SetArgPointee<3>(error ? asapo::IOErrorTemplates::kConnectionRefused.Generate().release() : nullptr),
                  SaveArg1ToGenericNetworkResponse(response),
                  Return(0)
                 ));
        if (error) {
            EXPECT_CALL(mock_logger, Error(AllOf(HasSubstr("error sending response"), HasSubstr(connected_uri))));
        }

        return;
    }
};


TEST_F(RequestsDispatcherTests, ErrorReceivetNextRequest) {
    EXPECT_CALL(mock_statictics, StartTimer_t(StatisticEntity::kNetwork));
    MockReceiveRequest(true);

    Error err;
    dispatcher->GetNextRequest(&err);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}


TEST_F(RequestsDispatcherTests, ClosedConnectionOnReceivetNextRequest) {
    EXPECT_CALL(mock_statictics, StartTimer_t(StatisticEntity::kNetwork));
    EXPECT_CALL(mock_io, Receive_t(_, _, _, _))
    .WillOnce(
        DoAll(SetArgPointee<3>(asapo::GeneralErrorTemplates::kEndOfFile.Generate().release()),
              Return(0))
    );
    Error err;
    dispatcher->GetNextRequest(&err);

    ASSERT_THAT(err, Eq(asapo::GeneralErrorTemplates::kEndOfFile));
}



TEST_F(RequestsDispatcherTests, ErrorCreatetNextRequest) {
    MockReceiveRequest(false);
    MockCreateRequest(true);

    Error err;
    dispatcher->GetNextRequest(&err);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kInvalidOpCode));
}

TEST_F(RequestsDispatcherTests, OkCreatetNextRequest) {
    MockReceiveRequest(false);
    MockCreateRequest(false);

    Error err;
    dispatcher->GetNextRequest(&err);

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(RequestsDispatcherTests, ErrorProcessRequestErrorSend) {
    MockHandleRequest(1);
    MockSendResponse(&response, true);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kUnknownIOError));
}


TEST_F(RequestsDispatcherTests, OkProcessRequestErrorSend) {
    MockHandleRequest(0);
    MockSendResponse(&response, true);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(asapo::IOErrorTemplates::kConnectionRefused));
}


TEST_F(RequestsDispatcherTests, OkProcessRequestSendOK) {
    MockHandleRequest(0);
    MockSendResponse(&response, false);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(RequestsDispatcherTests, ProcessRequestReturnsOkWithWarning) {
    MockHandleRequest(0);
    MockSendResponse(&response, false);
    request->SetResponseMessage("duplicate", asapo::ResponseMessageType::kWarning);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(response.error_code, Eq(asapo::kNetErrorWarning));
    ASSERT_THAT(std::string(response.message), HasSubstr(std::string("duplicate")));
}

TEST_F(RequestsDispatcherTests, ProcessRequestReturnsOkWithInfo) {
    MockHandleRequest(0);
    MockSendResponse(&response, false);
    request->SetResponseMessage("some info", asapo::ResponseMessageType::kInfo);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(nullptr));
    ASSERT_THAT(response.error_code, Eq(asapo::kNetErrorNoError));
    ASSERT_THAT(std::string(response.message), HasSubstr(std::string("some info")));
}

TEST_F(RequestsDispatcherTests, ProcessRequestReturnsAuthorizationFailure) {
    MockHandleRequest(1, asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate());
    MockSendResponse(&response, false);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kAuthorizationFailure));
    ASSERT_THAT(response.error_code, Eq(asapo::kNetAuthorizationError));
    ASSERT_THAT(std::string(response.message), HasSubstr("authorization"));
}

TEST_F(RequestsDispatcherTests, ProcessRequestReturnsUnsupportedClientFailure) {
    MockHandleRequest(1, asapo::ReceiverErrorTemplates::kUnsupportedClient.Generate());
    MockSendResponse(&response, false);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kUnsupportedClient));
    ASSERT_THAT(response.error_code, Eq(asapo::kNetErrorNotSupported));
    ASSERT_THAT(std::string(response.message), HasSubstr("supported"));
}

TEST_F(RequestsDispatcherTests, ProcessRequestReturnsReAuthorizationFailure) {
    MockHandleRequest(2, asapo::ReceiverErrorTemplates::kReAuthorizationFailure.Generate());
    MockSendResponse(&response, false);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kReAuthorizationFailure));
    ASSERT_THAT(response.error_code, Eq(asapo::kNetErrorReauthorize));
    ASSERT_THAT(std::string(response.message), HasSubstr("reauthorization"));
}


TEST_F(RequestsDispatcherTests, ProcessRequestReturnsBadRequest) {
    MockHandleRequest(1, asapo::ReceiverErrorTemplates::kBadRequest.Generate());
    MockSendResponse(&response, false);

    auto err = dispatcher->ProcessRequest(request);

    ASSERT_THAT(response.error_code, Eq(asapo::kNetErrorWrongRequest));
}



}
