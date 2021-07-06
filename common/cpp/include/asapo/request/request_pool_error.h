#ifndef ASAPO_REQUEST_POOL_ERROR_H
#define ASAPO_REQUEST_POOL_ERROR_H

#include "asapo/common/error.h"

namespace asapo {

class OriginalRequest : public CustomErrorData {
  public:
    GenericRequestPtr request;
};

}

#endif //ASAPO_REQUEST_POOL_ERROR_H
