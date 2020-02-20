#ifndef ASAPO_REQUEST_HANDLER_FILESYSTEM_H
#define ASAPO_REQUEST_HANDLER_FILESYSTEM_H

#include <chrono>

#include "io/io.h"
#include "common/error.h"

#include "producer/common.h"
#include "request/request_handler.h"
#include "logger/logger.h"

using std::chrono::system_clock;

namespace asapo {

class RequestHandlerFilesystem: public RequestHandler {
  public:
    explicit RequestHandlerFilesystem(std::string destination_folder, uint64_t thread_id);
    bool ProcessRequestUnlocked(GenericRequest* request) override;
    bool ReadyProcessRequest() override {
        return true;
    };
    void PrepareProcessingRequestLocked()  override {};
    void TearDownProcessingRequestLocked(bool processing_succeeded)  override {};
    void ProcessRequestTimeout(GenericRequest* request)  override;

    virtual ~RequestHandlerFilesystem() = default;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    std::string destination_folder_;
    uint64_t thread_id_;
};
}

#endif //ASAPO_REQUEST_HANDLER_FILESYSTEM_H
