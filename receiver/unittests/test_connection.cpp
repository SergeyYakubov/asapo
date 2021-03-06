#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/common/error.h"
#include "asapo/unittests/MockLogger.h"
#include "../src/connection.h"
#include "receiver_mocking.h"
#include "../src/receiver_config.h"

using namespace testing;
using namespace asapo;

namespace {

TEST(Connection, Constructor) {
    Connection connection{0, "some_address", nullptr, "some_tag"};
    ASSERT_THAT(dynamic_cast<asapo::Statistics*>(connection.statistics__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::IO*>(connection.io__.get()), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<const asapo::AbstractLogger*>(connection.log__), Ne(nullptr));
    ASSERT_THAT(dynamic_cast<asapo::RequestsDispatcher*>(connection.requests_dispatcher__.get()), Ne(nullptr));
}


class MockDispatcher: public asapo::RequestsDispatcher {
  public:
    MockDispatcher(): asapo::RequestsDispatcher(0, "", nullptr, nullptr) {};
    Error ProcessRequest(const std::unique_ptr<Request>& request) const noexcept override {
        return Error{ProcessRequest_t(request.get())};
    }

    std::unique_ptr<Request> GetNextRequest(Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto req = GetNextRequest_t(&error);
        err->reset(error);
        return std::unique_ptr<Request> {req};
    };

    MOCK_CONST_METHOD1(ProcessRequest_t, ErrorInterface * (Request*));
    MOCK_CONST_METHOD1(GetNextRequest_t, Request * (asapo::ErrorInterface**));

};


class ConnectionTests : public Test {
  public:
    std::string connected_uri{"some_address"};
    NiceMock<MockIO> mock_io;
    MockDispatcher mock_dispatcher;
    NiceMock<MockStatistics> mock_statictics;
    NiceMock<asapo::MockLogger> mock_logger;
    std::unique_ptr<Connection> connection;

    void SetUp() override {
        connection = std::unique_ptr<Connection> {new Connection{0, connected_uri, nullptr, "some_tag"}};
        connection->io__ = std::unique_ptr<asapo::IO> {&mock_io};
        connection->statistics__ = std::unique_ptr<asapo::ReceiverStatistics> {&mock_statictics};
        connection->log__ = &mock_logger;
        connection->requests_dispatcher__ = std::unique_ptr<asapo::RequestsDispatcher> {&mock_dispatcher};
        EXPECT_CALL(mock_io, CloseSocket_t(_, _));
        EXPECT_CALL(mock_statictics, SendIfNeeded_t(true));
        EXPECT_CALL(mock_logger, Info(HasSubstr("disconnected")));

    }
    void TearDown() override {
        connection->io__.release();
        connection->statistics__.release();
        connection->requests_dispatcher__.release();
    }

    Request* MockGetNext(bool error) {
        if (error ) {
            EXPECT_CALL(mock_dispatcher, GetNextRequest_t(_))
            .WillOnce(DoAll(
                          SetArgPointee<0>(asapo::GeneralErrorTemplates::kSimpleError.Generate("error").release()),
                          Return(nullptr)
                      ));
            return nullptr;
        } else {
            auto request = new Request(GenericRequestHeader{asapo::kOpcodeUnknownOp, 0, 1, 0, ""}, 0, connected_uri, nullptr,
                                       nullptr);
            EXPECT_CALL(mock_dispatcher, GetNextRequest_t(_))
            .WillOnce(DoAll(
                          SetArgPointee<0>(nullptr),
                          Return(request)
                      ));
            return request;
        }
    }

    void MockProcessRequest(Request* request, bool error) {
        if (error ) {
            EXPECT_CALL(mock_dispatcher, ProcessRequest_t(request))
            .WillOnce(
                Return(asapo::GeneralErrorTemplates::kSimpleError.Generate("error").release())
            );
        } else {
            EXPECT_CALL(mock_dispatcher, ProcessRequest_t(request))
            .WillOnce(
                Return(nullptr)
            );
        }
    }

};


TEST_F(ConnectionTests, ExitOnErrorsWithGetNextRequest) {
    MockGetNext(true);

    connection->Listen();
}


TEST_F(ConnectionTests, ProcessStatisticsWhenOKProcessRequest) {
    InSequence sequence;
    auto request = MockGetNext(false);

    MockProcessRequest(request, false);

    EXPECT_CALL(mock_statictics, IncreaseRequestCounter_t());
    EXPECT_CALL(mock_statictics, IncreaseRequestDataVolume_t(1 + sizeof(asapo::GenericRequestHeader) +
                sizeof(asapo::GenericNetworkResponse)));
    EXPECT_CALL(mock_statictics, SendIfNeeded_t(false));


    MockGetNext(true);

    connection->Listen();
}


TEST_F(ConnectionTests, ExitOnErrorsWithProcessRequest) {
    auto request = MockGetNext(false);

    MockProcessRequest(request, true);

    connection->Listen();
}


}
