#ifndef ASAPO_REQUEST_HANDLER_FACTORY_H
#define ASAPO_REQUEST_HANDLER_FACTORY_H

#include <memory>

#include "request_handler.h"

namespace  asapo {

class RequestHandlerFactory {
  public:
    virtual std::unique_ptr<RequestHandler> NewRequestHandler(uint64_t thread_id, uint64_t* shared_counter) = 0;
    virtual ~RequestHandlerFactory(){};
};


}

#endif //ASAPO_REQUEST_HANDLER_FACTORY_H
