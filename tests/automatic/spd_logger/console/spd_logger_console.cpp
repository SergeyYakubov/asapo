#include <thread>
#include <vector>

#include "logger/logger.h"

using namespace hidra2;

int main(int argc, char* argv[]) {

    auto logger = CreateDefaultLoggerBin("test_logger");

    logger->SetLogLevel(LogLevel::Debug);

    auto exec = [&](const std::string & i) {
        logger->Info("test info_mt_" + i);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(std::thread(exec, std::to_string(i)));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    logger->Info("test info");
    logger->Error("test error");
    logger->Warning("test warning");
    logger->Debug("test debug");


    logger->SetLogLevel(LogLevel::Error);
    logger->Info("test info_errorlev");
    logger->Error("test error_errorlev");
    logger->Warning("test warning_errorlev");
    logger->Debug("test debug_errorlev");


    logger->SetLogLevel(LogLevel::Warning);
    logger->Info("test info_warninglev");
    logger->Error("test error_warninglev");
    logger->Warning("test warning_warninglev");
    logger->Debug("test debug_warninglev");

    logger->SetLogLevel(LogLevel::Info);
    logger->Info("test info_infolev");
    logger->Error("test error_infolev");
    logger->Warning("test warning_infolev");
    logger->Debug("test debug_infolev");

    logger->SetLogLevel(LogLevel::None);
    logger->Info("test info_nonelev");
    logger->Error("test error_nonelev");
    logger->Warning("test warning_nonelev");
    logger->Debug("test debug_nonelev");


    return 0;
}
