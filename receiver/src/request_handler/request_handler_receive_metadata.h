#ifndef ASAPO_REQUEST_HANDLER_RECEIVE_METADATA_H
#define ASAPO_REQUEST_HANDLER_RECEIVE_METADATA_H

#include "request_handler.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerReceiveMetaData final: public ReceiverRequestHandler {
  public:
    RequestHandlerReceiveMetaData();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
};

}

#endif //ASAPO_REQUEST_HANDLER_RECEIVE_METADATA_H
