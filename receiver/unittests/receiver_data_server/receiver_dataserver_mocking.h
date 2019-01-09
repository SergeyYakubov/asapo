#ifndef ASAPO_MOCK_STATISTICS_H
#define ASAPO_MOCK_STATISTICS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../src/receiver_data_server/net_server.h"
#include "../../src/receiver_data_server/request_pool.h"


namespace asapo {

class MockNetServer : public NetServer {
  public:
    Requests GetNewRequests(Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto reqs = GetNewRequests_t(&error);
        err->reset(error);
        return  reqs;
    }

    MOCK_CONST_METHOD1(GetNewRequests_t, Requests (ErrorInterface**
                                                   error));
};

class MockPool : public RequestPool {
  public:
    Error AddRequests(const Requests& requests) noexcept override {
        return Error(AddRequests_t(requests));

    }

    MOCK_METHOD1(AddRequests_t, ErrorInterface * (const Requests&));
};



}

#endif //ASAPO_MOCK_STATISTICS_H
