#ifndef ASAPO_REQUEST_HANDLER_FILE_WRITE_H
#define ASAPO_REQUEST_HANDLER_FILE_WRITE_H

#include "request_handler.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerFileWrite final: public ReceiverRequestHandler {
  public:
    RequestHandlerFileWrite();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
};

}

#endif //ASAPO_REQUEST_HANDLER_FILE_WRITE_H
