#ifndef ASAPO_MOCKING_H
#define ASAPO_MOCKING_H

#include <gtest/gtest.h>

#include "../../include/request/request_pool.h"
#include "../../include/request/request_handler_factory.h"

namespace asapo {

const std::string expected_endpoint = "expected_endpont";

class MockRequestHandler : public RequestHandler {
  public:
    MOCK_METHOD0(PrepareProcessingRequestLocked, void());
    MOCK_METHOD0(ReadyProcessRequest, bool());
    MOCK_METHOD1(TearDownProcessingRequestLocked, void(bool processing_succeeded));
    MOCK_METHOD1(ProcessRequestUnlocked_t, bool (const GenericRequest* request));
    MOCK_METHOD1(ProcessRequestTimeout, void(GenericRequest* request));
    uint64_t retry_counter = 0;
    bool ProcessRequestUnlocked(GenericRequest* request)  override {
        retry_counter = request->GetRetryCounter();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return ProcessRequestUnlocked_t(request);
    }


};

}

using asapo::MockRequestHandler;

#endif //ASAPO_MOCKING_H
