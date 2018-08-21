#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock_foldermon_config.h"
#include "../src/foldermon_config_factory.h"
#include "../src/foldermon_config.h"

#include <unittests/MockIO.h>

using testing::_;

namespace asapo {

Error SetFolderMonConfig (const FolderMonConfig& config) {
    MockIO mock_io;
    FolderMonConfigFactory config_factory;
    config_factory.io__ = std::unique_ptr<IO> {&mock_io};

    std::string log_level;
    switch (config.log_level) {
    case LogLevel::Error:
        log_level = "error";
        break;
    case LogLevel::Warning:
        log_level = "warning";
        break;
    case LogLevel::Info:
        log_level = "info";
        break;
    case LogLevel::Debug:
        log_level = "debug";
        break;
    case LogLevel::None:
        log_level = "none";
        break;
    }

    std::string mode = "tcp";
    if (config.mode == RequestHandlerType::kFilesystem) {
        mode = "filesystem";
    }

    if (config.asapo_endpoint == "wrongmode") {
        mode = "bla";
    }

    auto config_string = std::string("{\"BeamtimeID\":") + "\"" + config.beamtime_id + "\"";
    config_string += "," + std::string("\"Mode\":") + "\"" + mode + "\"";
    config_string += "," + std::string("\"NThreads\":") + std::to_string(config.nthreads);
    config_string += "," + std::string("\"LogLevel\":") + "\"" + log_level + "\"";
    config_string += "," + std::string("\"Tag\":") + "\"" + config.tag + "\"";
    config_string += "," + std::string("\"AsapoEndpoint\":") + "\"" + config.asapo_endpoint + "\"";

    config_string += "}";


    EXPECT_CALL(mock_io, ReadFileToString_t("fname", _)).WillOnce(
        testing::Return(config_string)
    );

    auto err = config_factory.SetConfigFromFile("fname");

    config_factory.io__.release();

    return err;
}

}


