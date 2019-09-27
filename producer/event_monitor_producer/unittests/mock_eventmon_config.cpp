#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock_eventmon_config.h"
#include "../src/eventmon_config_factory.h"
#include "../src/eventmon_config.h"

#include <unittests/MockIO.h>

using testing::_;

namespace asapo {

Error SetFolderMonConfig (const EventMonConfig& config) {
    MockIO mock_io;
    EventMonConfigFactory config_factory;
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
    config_string += "," + std::string("\"RemoveAfterSend\":") + (config.remove_after_send ? "true" : "false");
    config_string += "," + std::string("\"Stream\":") + "\"" + config.stream + "\"";

    std::string subset_mode;
    switch (config.subset_mode) {
    case SubSetMode::kBatch:
        subset_mode = "batch";
        break;
    case SubSetMode::kMultiSource:
        subset_mode = "multisource";
        break;

    case SubSetMode::kNone:
        subset_mode = "none";
        break;

    }
    config_string += "," + std::string("\"Subset\":{");
    config_string += std::string("\"Mode\":") + "\"" + subset_mode + "\"";
    if (config.subset_mode == SubSetMode::kBatch) {
        config_string += "," + std::string("\"BatchSize\":") + std::to_string(config.subset_batch_size);
    }
    if (config.subset_mode == SubSetMode::kMultiSource) {
        config_string += "," + std::string("\"SourceId\":") + std::to_string(config.subset_multisource_sourceid);
        config_string += "," + std::string("\"NSources\":") + std::to_string(config.subset_multisource_nsources);
    }

    config_string += "}";

    std::string mon_folders;
    for (auto folder : config.monitored_subfolders) {
        mon_folders += "\"" + folder + "\"" + ",";
    }
    if (mon_folders.size()) {
        mon_folders.pop_back();
    }

    std::string ignored_exts;
    for (auto ext : config.ignored_extentions) {
        ignored_exts += "\"" + ext + "\"" + ",";
    }
    if (ignored_exts.size()) {
        ignored_exts.pop_back();
    }


    config_string += "," + std::string("\"MonitoredSubFolders\":") + "[" + mon_folders + "]";
    config_string += "," + std::string("\"RootMonitoredFolder\":") + "\"" + config.root_monitored_folder + "\"";
    config_string += "," + std::string("\"IgnoreExtentions\":") + "[" + ignored_exts + "]";
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


