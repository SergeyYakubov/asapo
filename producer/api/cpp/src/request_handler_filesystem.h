#ifndef ASAPO_REQUEST_HANDLER_FILESYSTEM_H
#define ASAPO_REQUEST_HANDLER_FILESYSTEM_H

#include <chrono>

#include "asapo/io/io.h"
#include "asapo/common/error.h"

#include "asapo/producer/common.h"
#include "asapo/request/request_handler.h"
#include "asapo/logger/logger.h"

using std::chrono::system_clock;

namespace asapo {

class RequestHandlerFilesystem: public RequestHandler {
  public:
    explicit RequestHandlerFilesystem(std::string destination_folder, uint64_t thread_id);
    bool ProcessRequestUnlocked(GenericRequest* request, bool* retry) override;
    bool ReadyProcessRequest() override {
        return true;
    };
    void PrepareProcessingRequestLocked()  override {};
    void TearDownProcessingRequestLocked(bool )  override {};
    void ProcessRequestTimeoutUnlocked(GenericRequest* request)  override;

    virtual ~RequestHandlerFilesystem() = default;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    std::string destination_folder_;
};
}

#endif //ASAPO_REQUEST_HANDLER_FILESYSTEM_H
