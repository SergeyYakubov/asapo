#ifndef ASAPO_REQUEST_HANDLER_RECEIVE_DATA_H
#define ASAPO_REQUEST_HANDLER_RECEIVE_DATA_H

#include "request_handler.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerReceiveData final: public ReceiverRequestHandler {
  public:
    RequestHandlerReceiveData();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
 private:
  Error ReceiveData(Request* request)const;
  Error ReceiveMetaData(Request* request) const;
  Error ReceiveRequestContent(Request* request) const;
  bool NeedReceiveData(const Request* request) const;

};

}

#endif //ASAPO_REQUEST_HANDLER_RECEIVE_DATA_H
