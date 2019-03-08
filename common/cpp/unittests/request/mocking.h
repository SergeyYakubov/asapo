#ifndef ASAPO_MOCKING_H
#define ASAPO_MOCKING_H

#include <gtest/gtest.h>

#include "../../include/request/request_pool.h"
#include "../../include/request/request_handler_factory.h"

namespace asapo {

const std::string expected_endpoint = "expected_endpont";

class MockRequestHandler : public RequestHandler {
  public:

    Error ProcessRequestUnlocked(GenericRequest* request) override {
        return Error{ProcessRequestUnlocked_t(request)};
    }
    void TearDownProcessingRequestLocked(const Error& error_from_process) override {
        if (error_from_process) {
            TearDownProcessingRequestLocked_t(error_from_process.get());
        } else {
            TearDownProcessingRequestLocked_t(nullptr);
        }
    }
    MOCK_METHOD0(PrepareProcessingRequestLocked, void());
    MOCK_METHOD0(ReadyProcessRequest, bool());
    MOCK_METHOD1(TearDownProcessingRequestLocked_t, void(ErrorInterface* error_from_process));
    MOCK_METHOD1(ProcessRequestUnlocked_t, ErrorInterface * (const GenericRequest*));
};

}

using asapo::MockRequestHandler;

#endif //ASAPO_MOCKING_H
