#ifndef ASAPO_PRODUCER_REQUEST_HANDLER_H
#define ASAPO_PRODUCER_REQUEST_HANDLER_H

#include <memory>

#include "common/error.h"
#include "request.h"

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#endif


namespace  asapo {

class RequestHandler {
 public:
  virtual void PrepareProcessingRequestLocked()  = 0;
  virtual void TearDownProcessingRequestLocked(const Error &error_from_process)  = 0;
  virtual Error ProcessRequestUnlocked(const Request* request)  = 0;
  virtual bool ReadyProcessRequest() = 0;
  virtual ~RequestHandler() = default;
};


}
#endif //ASAPO_PRODUCER_REQUEST_HANDLER_H
