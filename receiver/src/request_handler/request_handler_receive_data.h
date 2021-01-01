#ifndef ASAPO_REQUEST_HANDLER_RECEIVE_DATA_H
#define ASAPO_REQUEST_HANDLER_RECEIVE_DATA_H

#include "request_handler.h"
#include "asapo/logger/logger.h"

#include "asapo/io/io.h"

namespace asapo {

class RequestHandlerReceiveData final: public ReceiverRequestHandler {
  public:
    RequestHandlerReceiveData();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    bool NeedReceiveData(const Request* request) const;

};

}

#endif //ASAPO_REQUEST_HANDLER_RECEIVE_DATA_H
