#ifndef ASAPO_MOCKING_H
#define ASAPO_MOCKING_H

#include <gtest/gtest.h>

#include "asapo/request/request_pool.h"
#include "asapo/request/request_handler_factory.h"
#include "../src/receiver_discovery_service.h"

namespace asapo {

const std::string expected_endpoint = "expected_endpont";

class MockDiscoveryService : public asapo::ReceiverDiscoveryService {
  public:
    MockDiscoveryService() : ReceiverDiscoveryService{expected_endpoint, 1} {};
    MOCK_METHOD0(StartCollectingData, void());
    MOCK_METHOD0(MaxConnections, uint64_t());
    MOCK_METHOD1(RotatedUriList, ReceiversList(uint64_t));
    uint64_t UpdateFrequency() override {
        return 0;
    }
};

class MockRequestPool : public RequestPool {
  public:
    MockRequestPool(RequestHandlerFactory* request_handler_factory, AbstractLogger* log) :
        RequestPool{1, request_handler_factory, log} {};
    asapo::Error AddRequest(std::unique_ptr<asapo::GenericRequest> request, bool top_priority = false) override {
        if (request == nullptr) {
            return asapo::Error{AddRequest_t(nullptr, top_priority)};
        }
        return asapo::Error{AddRequest_t(request.get(), top_priority)};
    }
    MOCK_METHOD2(AddRequest_t, asapo::ErrorInterface * (GenericRequest*, bool));
    MOCK_METHOD0(NRequestsInPool, uint64_t ());
    MOCK_METHOD0(UsedMemoryInPool, uint64_t ());

    MOCK_METHOD1(SetLimits, void (asapo::RequestPoolLimits limits));

    MOCK_METHOD1(WaitRequestsFinished_t, asapo::ErrorInterface * (uint64_t timeout_ms));

    asapo::Error WaitRequestsFinished(uint64_t timeout_ms) override {
        return asapo::Error{WaitRequestsFinished_t(timeout_ms)};
    }



};


}

using asapo::MockDiscoveryService;
using asapo::MockRequestPool;

#endif //ASAPO_MOCKING_H
