#include "asapo/logger/logger.h"

using namespace asapo;

int main() {

    auto logger = CreateDefaultLoggerApi("test_central", "http://localhost:9880/asapo");

    logger->SetLogLevel(LogLevel::Debug);

    logger->Info(LogMessageWithFields{"json_test", "info"});
    logger->Error("test error");
    logger->Warning("test warning");
    logger->Debug("test debug");

    return 0;
}
