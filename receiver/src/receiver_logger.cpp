#include "receiver_logger.h"

namespace hidra2 {


AbstractLogger* GetDefaultReceiverLogger() {
    static Logger logger = hidra2::CreateDefaultLoggerBin("receiver");
    return logger.get();
}

}
