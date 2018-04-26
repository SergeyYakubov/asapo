#include <thread>
#include <vector>

#include "logger/logger.h"

using namespace hidra2;

int main(int argc, char* argv[]) {

    auto logger = CreateDefaultLoggerBin("test_logger");

    logger->SetLogLevel(LogLevel::Debug);

    auto exec = [&](const std::string & i) {
        logger->Info("test_info_mt_" + i);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(std::thread(exec, std::to_string(i)));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    logger->Info("test_info");
    logger->Error("test_error");
    logger->Warning("test_warning");
    logger->Debug("test_debug");


    logger->SetLogLevel(LogLevel::Error);
    logger->Info("test_info_errorlev");
    logger->Error("test_error_errorlev");
    logger->Warning("test_warning_errorlev");
    logger->Debug("test_debug_errorlev");


    logger->SetLogLevel(LogLevel::Warning);
    logger->Info("test_info_warninglev");
    logger->Error("test_error_warninglev");
    logger->Warning("test_warning_warninglev");
    logger->Debug("test_debug_warninglev");

    logger->SetLogLevel(LogLevel::Info);
    logger->Info("test_info_infolev");
    logger->Error("test_error_infolev");
    logger->Warning("test_warning_infolev");
    logger->Debug("test_debug_infolev");

    logger->SetLogLevel(LogLevel::None);
    logger->Info("test_info_nonelev");
    logger->Error("test_error_nonelev");
    logger->Warning("test_warning_nonelev");
    logger->Debug("test_debug_nonelev");


    return 0;
}
