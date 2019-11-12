#ifndef ASAPO_REQUEST_HANDLER_FILE_RECEIVE_H
#define ASAPO_REQUEST_HANDLER_FILE_RECEIVE_H

#include "request_handler.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerFileReceive final: public ReceiverRequestHandler {
  public:
    RequestHandlerFileReceive();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
};

}

#endif //ASAPO_REQUEST_HANDLER_FILE_RECEIVE_H
