#include "receiver_logger.h"

namespace asapo {


AbstractLogger* GetDefaultReceiverLogger() {
    static Logger logger = asapo::CreateDefaultLoggerBin("receiver");
    return logger.get();
}

}
