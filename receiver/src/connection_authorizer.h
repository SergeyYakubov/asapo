#ifndef ASAPO_RECEIVER_AUTHORIZER_H
#define ASAPO_RECEIVER_AUTHORIZER_H

#include "common/error.h"
#include "preprocessor/definitions.h"
#include "logger/logger.h"

namespace asapo {

class ConnectionAuthorizer {
 public:
    ConnectionAuthorizer();
    VIRTUAL Error Authorize(std::string beamtime_id,std::string uri) const noexcept;
    const AbstractLogger* log__;
 private:
};

}

#endif //ASAPO_RECEIVER_AUTHORIZER_H
