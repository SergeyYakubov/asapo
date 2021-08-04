#ifndef ASAPO_RECEIVER_DATASERVER_MOCKING_H
#define ASAPO_RECEIVER_DATASERVER_MOCKING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../src/receiver_data_server/net_server/rds_net_server.h"
#include "asapo/request/request_pool.h"
#include "../../src/receiver_data_server/receiver_data_server_request.h"

namespace asapo {

class MockNetServer : public RdsNetServer {
  public:
    Error Initialize() override {
        return  Error{Initialize_t()};
    };
    MOCK_METHOD0(Initialize_t, ErrorInterface * ());

    GenericRequests GetNewRequests(Error* err) override {
        ErrorInterface* error = nullptr;
        auto reqs = GetNewRequests_t(&error);
        err->reset(error);
        GenericRequests res;
        for (const auto& preq : reqs) {
            ReceiverDataServerRequestPtr ptr = ReceiverDataServerRequestPtr{new ReceiverDataServerRequest{preq.header, preq.source_id, SharedInstancedStatistics{ new InstancedStatistics } }};
            res.push_back(std::move(ptr));
        }
        return  res;
    }

    MOCK_METHOD1(GetNewRequests_t, std::vector<ReceiverDataServerRequest> (ErrorInterface**
                 error));

    Error SendResponse(const ReceiverDataServerRequest* request,
                       const GenericNetworkResponse* response) override  {
        return  Error{SendResponse_t(request, response)};
    };
    MOCK_METHOD2(SendResponse_t, ErrorInterface * (const ReceiverDataServerRequest* request,
                                                   const GenericNetworkResponse* response));

    Error SendResponseAndSlotData(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response,
                                  const CacheMeta* cache_slot) override {
        return  Error{SendResponseAndSlotData_t(request, response, cache_slot)};
    };
    MOCK_METHOD3(SendResponseAndSlotData_t, ErrorInterface * (const ReceiverDataServerRequest* request,
                 const GenericNetworkResponse* response,
                 const CacheMeta* cache_slot));

    void  HandleAfterError(uint64_t source_id) override {
        HandleAfterError_t(source_id);
    }

    MOCK_METHOD1(HandleAfterError_t, void (uint64_t source_id));

    MOCK_METHOD0(Monitoring, SharedReceiverMonitoringClient());
};

class MockPool : public RequestPool {
  public:
    MockPool(): RequestPool(0, nullptr, nullptr) {};
    Error AddRequests(GenericRequests requests) noexcept override {
        std::vector<GenericRequest> reqs;
        for (const auto& preq : requests) {
            reqs.emplace_back(preq->header, 0);
        }
        return Error(AddRequests_t(std::move(reqs)));

    }

    MOCK_METHOD1(AddRequests_t, ErrorInterface * (std::vector<GenericRequest>));
};



}

#endif //ASAPO_RECEIVER_DATASERVER_MOCKING_H
