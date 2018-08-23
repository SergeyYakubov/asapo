#include "eventmon_logger.h"

namespace asapo {

AbstractLogger* GetDefaultEventMonLogger() {
    static Logger logger = asapo::CreateDefaultLoggerBin("producer ");
    return logger.get();
}

}
