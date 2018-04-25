#include <thread>
#include <vector>

#include "logger/logger.h"

using namespace hidra2;

int main(int argc, char* argv[]) {

    auto logger = CreateDefaultLoggerApi("test_central","http://localhost:9880/asapo");

    logger->SetLogLevel(LogLevel::Debug);

    logger->Info("test_info");
    logger->Error("test_error");
    logger->Warning("test_warning");
    logger->Debug("test_debug");

    return 0;
}
