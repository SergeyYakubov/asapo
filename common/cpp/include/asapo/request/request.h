#ifndef ASAPO_GENERIC_REQUEST_H
#define ASAPO_GENERIC_REQUEST_H

#include <chrono>

#include "asapo/common/networking.h"
#include "asapo/common/data_structs.h"
#include "asapo/io/io.h"

namespace asapo {

class GenericRequest {
  public:
    GenericRequest() = delete;
    GenericRequest(GenericRequestHeader h, uint64_t timeout_ms): header{std::move(h)}, timeout_ms_{timeout_ms} {};
    GenericRequestHeader header;
    virtual ~GenericRequest() = default;
    uint64_t GetRetryCounter() {
        return retry_counter_;
    }
    void IncreaseRetryCounter() {
        retry_counter_++;
    }
    bool TimedOut() {
        if (timeout_ms_ == 0) {
            return false;
        }
        uint64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() -
                              created_at_).count();
        return elapsed_ms > timeout_ms_;
    }

  private:
    uint64_t retry_counter_ = 0;
    uint64_t timeout_ms_ = 0;
    std::chrono::system_clock::time_point created_at_ = std::chrono::system_clock::now();
};

using GenericRequestPtr = std::unique_ptr<GenericRequest>;
using GenericRequests = std::vector<GenericRequestPtr>;

}

#endif //ASAPO_GENERIC_REQUEST_H
