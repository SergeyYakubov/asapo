#ifndef ASAPO_RECEIVER_LOGGER_H
#define ASAPO_RECEIVER_LOGGER_H

#include "asapo/logger/logger.h"
#include "request.h"

namespace asapo {

AbstractLogger* GetDefaultReceiverLogger();
LogMessageWithFields RequestLog(std::string message, const Request* request);

}


#endif //ASAPO_RECEIVER_LOGGER_H
