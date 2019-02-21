#ifndef ASAPO_MOCK_STATISTICS_H
#define ASAPO_MOCK_STATISTICS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../src/receiver_data_server/net_server.h"
#include "request/request_pool.h"
#include "../../src/receiver_data_server/receiver_data_server_request.h"

namespace asapo {

class MockNetServer : public NetServer {
  public:
    GenericRequests GetNewRequests(Error* err) const noexcept override {
        ErrorInterface* error = nullptr;
        auto reqs = GetNewRequests_t(&error);
        err->reset(error);
        GenericRequests res;
        for (const auto& preq : reqs) {
            ReceiverDataServerRequestPtr ptr = ReceiverDataServerRequestPtr{new ReceiverDataServerRequest{preq.header, preq.net_id, preq.server}};
            res.push_back(std::move(ptr));
        }
        return  res;
    }

    MOCK_CONST_METHOD1(GetNewRequests_t, std::vector<ReceiverDataServerRequest> (ErrorInterface**
                       error));
};

class MockPool : public RequestPool {
  public:
    MockPool(): RequestPool(0, nullptr, nullptr) {};
    Error AddRequests(GenericRequests requests) noexcept override {
        std::vector<GenericRequest> reqs;
        for (const auto& preq : requests) {
            reqs.push_back(GenericRequest{preq->header});
        }
        return Error(AddRequests_t(std::move(reqs)));

    }

    MOCK_METHOD1(AddRequests_t, ErrorInterface * (std::vector<GenericRequest>));
};



}

#endif //ASAPO_MOCK_STATISTICS_H
