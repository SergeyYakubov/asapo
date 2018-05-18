#ifndef ASAPO_MOCKING_H
#define ASAPO_MOCKING_H

#include <gtest/gtest.h>

#include "../src/request_pool.h"
#include "../src/request_handler_factory.h"
#include "../src/receiver_discovery_service.h"

namespace asapo {

const std::string expected_endpoint="expected_endpont";

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


class MockRequestPull : public RequestPool {
 public:
  MockRequestPull(RequestHandlerFactory* request_handler_factory) :
      RequestPool{1, request_handler_factory} {};
  asapo::Error AddRequest(std::unique_ptr<asapo::Request> request) override {
      if (request == nullptr) {
          return asapo::Error{AddRequest_t(nullptr)};
      }
      return asapo::Error{AddRequest_t(request.get())};
  }
  MOCK_METHOD1(AddRequest_t, asapo::ErrorInterface * (Request*));
};


class MockRequestHandler : public RequestHandler {
 public:

  Error ProcessRequestUnlocked(const Request* request) override {
      return Error{ProcessRequestUnlocked_t(request)};
  }
  void TearDownProcessingRequestLocked(const Error& error_from_process) override{
      if (error_from_process) {
        TearDownProcessingRequestLocked_t(error_from_process.get());
      }
      else {
          TearDownProcessingRequestLocked_t(nullptr);
      }
  }
    MOCK_METHOD0(PrepareProcessingRequestLocked, void());
  MOCK_METHOD0(ReadyProcessRequest, bool());
  MOCK_METHOD1(TearDownProcessingRequestLocked_t, void(ErrorInterface* error_from_process));
  MOCK_METHOD1(ProcessRequestUnlocked_t, ErrorInterface*(const Request*));
};



}

using asapo::MockRequestHandler;
using asapo::MockDiscoveryService;
using asapo::MockRequestPull;

#endif //ASAPO_MOCKING_H
