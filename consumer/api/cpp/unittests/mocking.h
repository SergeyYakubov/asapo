#ifndef ASAPO_CONSUMER_MOCKING_H
#define ASAPO_CONSUMER_MOCKING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/net_client.h"
#include "../src/tcp_connection_pool.h"

namespace asapo {

class MockNetClient : public asapo::NetClient {
  public:

    Error GetData(const MessageMeta* info, const std::string& request_sender_details, MessageData* data) override {
        return Error(GetData_t(info, request_sender_details, data));
    }

    MOCK_CONST_METHOD3(GetData_t, ErrorInterface * (const MessageMeta* info, const std::string& request_sender_details, MessageData* data));

};


class MockTCPConnectionPool : public asapo::TcpConnectionPool {
  public:

    SocketDescriptor GetFreeConnection(const std::string& source, bool* reused, Error* err) override {
        ErrorInterface* error = nullptr;
        auto data = GetFreeConnection_t(source, reused, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD3(GetFreeConnection_t, SocketDescriptor (const std::string&, bool* reused, ErrorInterface**));

    SocketDescriptor Reconnect(SocketDescriptor sd, Error* err) override {
        ErrorInterface* error = nullptr;
        auto data = Reconnect_t(sd, &error);
        err->reset(error);
        return data;
    }
    MOCK_METHOD2(Reconnect_t, SocketDescriptor (SocketDescriptor, ErrorInterface**));

    MOCK_METHOD1(ReleaseConnection, void (SocketDescriptor));
};

}



#endif //ASAPO_CONSUMER_MOCKING_H
