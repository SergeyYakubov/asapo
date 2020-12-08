#ifndef ASAPO_REQUEST_HANDLER_H
#define ASAPO_REQUEST_HANDLER_H

#include <memory>

#include "asapo/common/error.h"

#include "request.h"

namespace  asapo {

class RequestHandler {
  public:
    virtual void PrepareProcessingRequestLocked()  = 0;
    virtual void TearDownProcessingRequestLocked(bool success)  = 0;
    virtual bool ProcessRequestUnlocked(GenericRequest* request, bool* retry)  = 0;
    virtual void ProcessRequestTimeout(GenericRequest* request)  = 0;
    virtual bool ReadyProcessRequest() = 0;
    virtual ~RequestHandler() = default;
};


}
#endif //ASAPO_REQUEST_HANDLER_H
