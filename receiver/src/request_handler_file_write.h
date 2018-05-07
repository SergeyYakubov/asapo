#ifndef HIDRA2_REQUEST_HANDLER_FILE_WRITE_H
#define HIDRA2_REQUEST_HANDLER_FILE_WRITE_H

#include "request_handler.h"
#include "logger/logger.h"

#include "io/io.h"

namespace hidra2 {

const uint64_t kMaxFileSize = uint64_t(1024) * 1024 * 1024 * 2; //2GB

class RequestHandlerFileWrite final: public RequestHandler {
  public:
    RequestHandlerFileWrite();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(const Request& request) const override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
};

}

#endif //HIDRA2_REQUEST_HANDLER_FILE_WRITE_H
