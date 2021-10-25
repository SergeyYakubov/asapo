#ifndef ASAPO_RECEIVER_LOGGER_H
#define ASAPO_RECEIVER_LOGGER_H

#include "asapo/logger/logger.h"

namespace asapo {

class Request;

AbstractLogger* GetDefaultReceiverLogger();
LogMessageWithFields RequestLog(std::string message, const Request* request, std::string origin);

}


#endif //ASAPO_RECEIVER_LOGGER_H
