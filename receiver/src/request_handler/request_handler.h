#ifndef ASAPO_RECEIVER_REQUEST_HANDLER_H
#define ASAPO_RECEIVER_REQUEST_HANDLER_H

#include "../receiver_error.h"
#include "../statistics/receiver_statistics.h"

namespace asapo {

class Request;

class ReceiverRequestHandler {
  public:
    virtual Error ProcessRequest(Request* request) const = 0;
    virtual StatisticEntity GetStatisticEntity() const  = 0;
    virtual ~ReceiverRequestHandler() = default;
  private:
};

}

#endif //ASAPO_RECEIVER_REQUEST_HANDLER_H
