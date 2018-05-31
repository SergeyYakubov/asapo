#ifndef ASAPO_REQUEST_HANDLER_FILESYSTEM_H
#define ASAPO_REQUEST_HANDLER_FILESYSTEM_H

#include <chrono>

#include "io/io.h"
#include "common/error.h"

#include "producer/common.h"
#include "request_handler.h"
#include "logger/logger.h"

using std::chrono::high_resolution_clock;

namespace asapo {

class RequestHandlerFilesystem: public RequestHandler {
  public:
    explicit RequestHandlerFilesystem(std::string destination_folder, uint64_t thread_id);
    Error ProcessRequestUnlocked(const Request* request) override;
    bool ReadyProcessRequest() override {
        return true;
    };
    void PrepareProcessingRequestLocked()  override {};
    void TearDownProcessingRequestLocked(const Error& error_from_process)  override {};

    virtual ~RequestHandlerFilesystem() = default;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    std::string destination_folder_;
    uint64_t thread_id_;
};
}

#endif //ASAPO_REQUEST_HANDLER_FILESYSTEM_H
