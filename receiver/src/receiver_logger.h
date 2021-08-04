#ifndef ASAPO_RECEIVER_LOGGER_H
#define ASAPO_RECEIVER_LOGGER_H

#include "asapo/logger/logger.h"

namespace asapo {


AbstractLogger* GetDefaultReceiverLogger();
AbstractLogger* GetDefaultReceiverMonitoringLogger();

}


#endif //ASAPO_RECEIVER_LOGGER_H
