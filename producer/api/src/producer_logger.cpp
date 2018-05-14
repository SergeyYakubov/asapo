#include "producer_logger.h"

namespace asapo {


AbstractLogger* GetDefaultProducerLogger() {
    //todo get fluentd uri from service discovery
    static Logger logger = CreateDefaultLoggerApi("producer_api", "http://max-wgs.desy.de:9880/asapo");
    return logger.get();
}

}
