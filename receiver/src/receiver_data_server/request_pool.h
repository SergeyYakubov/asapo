#ifndef ASAPO_REQUEST_POOL_H
#define ASAPO_REQUEST_POOL_H

#include "preprocessor/definitions.h"
#include "common/error.h"
#include "common.h"

namespace asapo {

class RequestPool {
  public:
    VIRTUAL Error AddRequests(const Requests& requests) noexcept;
};

}

#endif //ASAPO_REQUEST_POOL_H
