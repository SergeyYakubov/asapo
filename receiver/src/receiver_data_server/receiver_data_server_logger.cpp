#include "receiver_data_server_logger.h"

namespace asapo {


AbstractLogger* GetDefaultReceiverDataServerLogger() {
    static Logger logger = asapo::CreateDefaultLoggerBin("receiver_dataserver");
    return logger.get();
}

}
