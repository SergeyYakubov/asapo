#ifndef ASAPO_RECEIVER_LOGGER_H
#define ASAPO_RECEIVER_LOGGER_H

#include "asapo/logger/logger.h"

namespace asapo {

struct AuthorizationData;
class Request;

AbstractLogger* GetDefaultReceiverLogger();
AbstractLogger* GetDefaultReceiverMonitoringLogger();
LogMessageWithFields RequestLog(std::string message, const Request* request);
LogMessageWithFields AuthorizationLog(std::string message, const Request* request, const AuthorizationData* data);

}


#endif //ASAPO_RECEIVER_LOGGER_H
