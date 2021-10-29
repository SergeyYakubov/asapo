#include "eventmon_config.h"
#include "eventmon_config_factory.h"
#include "asapo/io/io_factory.h"
#include "asapo/json_parser/json_parser.h"

namespace asapo {

EventMonConfig config;

EventMonConfigFactory::EventMonConfigFactory() : io__{GenerateDefaultIO()} {

}

Error DatasetModeToEnum(const std::string& mode_str, DatasetMode* mode) {
    if (mode_str == "batch") {
        *mode = DatasetMode::kBatch;
        return nullptr;
    }

    if (mode_str == "none") {
        *mode = DatasetMode::kNone;
        return nullptr;
    }

    if (mode_str == "multisource") {
        *mode = DatasetMode::kMultiSource;
        return nullptr;
    }


    return GeneralErrorTemplates::kSimpleError.Generate("Wrone dataset mode:" + mode_str);
}

Error EventMonConfigFactory::ParseConfigFile(std::string file_name) {
    JsonFileParser parser(file_name, &io__);
    Error err = nullptr;
    std::string dataset_mode;

    (err = parser.GetString("AsapoEndpoint", &config.asapo_endpoint)) ||
    (err = parser.GetString("Tag", &config.tag)) ||
    (err = parser.GetString("BeamtimeID", &config.beamtime_id)) ||
    (err = parser.GetString("DataSource", &config.data_source)) ||
    (err = parser.GetString("Mode", &config.mode_str)) ||
    (err = parser.GetUInt64("NThreads", &config.nthreads)) ||
    (err = parser.GetString("RootMonitoredFolder", &config.root_monitored_folder)) ||
    (err = parser.GetString("LogLevel", &config.log_level_str)) ||
    (err = parser.GetBool("RemoveAfterSend", &config.remove_after_send)) ||
    (err = parser.GetArrayString("MonitoredSubFolders", &config.monitored_subfolders)) ||
    (err = parser.GetArrayString("IgnoreExtensions", &config.ignored_extensions)) ||
    (err = parser.GetArrayString("WhitelistExtensions", &config.whitelisted_extensions)) ||
    (err = parser.Embedded("Dataset").GetString("Mode", &dataset_mode)) ||
    (err = DatasetModeToEnum(dataset_mode, &config.dataset_mode));
    if (err) {
        return err;
    }

    if (config.dataset_mode == DatasetMode::kBatch) {
        err = parser.Embedded("Dataset").GetUInt64("BatchSize", &config.dataset_batch_size);
    }

    if (config.dataset_mode == DatasetMode::kMultiSource) {
        (err = parser.Embedded("Dataset").GetUInt64("NSources", &config.dataset_multisource_nsources)) ||
        ((err = parser.Embedded("Dataset").GetUInt64("SourceId", &config.dataset_multisource_sourceid)));
    }


    return err;
}


Error EventMonConfigFactory::CheckConfig() {
    Error err;
    (err = CheckMode()) ||
    (err = CheckLogLevel()) ||
    (err = CheckNThreads()) ||
    (err = CheckDatasets());

//todo: check monitored folders exist?
    return err;
}


Error EventMonConfigFactory::SetConfigFromFile(std::string file_name) {
    auto  err = ParseConfigFile(file_name);
    if (err) {
        return err;
    }

    return CheckConfig();
}

Error EventMonConfigFactory::CheckMode() {
    if (config.mode_str == "tcp") {
        config.mode = RequestHandlerType::kTcp;
    } else if (config.mode_str == "filesystem") {
        config.mode = RequestHandlerType::kFilesystem;
    } else {
        return  GeneralErrorTemplates::kSimpleError.Generate("wrong producer mode: " + config.mode_str);
    }
    return nullptr;
}

Error EventMonConfigFactory::CheckLogLevel() {
    Error err;
    config.log_level = StringToLogLevel(config.log_level_str, &err);
    return err;
}

Error EventMonConfigFactory::CheckNThreads() {
    if (config.nthreads == 0 || config.nthreads > kMaxProcessingThreads ) {
        return  GeneralErrorTemplates::kSimpleError.Generate("NThreads should between 1 and " + std::to_string(
                    kMaxProcessingThreads));
    }
    return nullptr;
}

Error EventMonConfigFactory::CheckDatasets() {
    if (config.dataset_mode == DatasetMode::kBatch && config.dataset_batch_size < 1) {
        return  GeneralErrorTemplates::kSimpleError.Generate("Batch size should > 0");
    }


    if (config.dataset_mode == DatasetMode::kMultiSource && config.dataset_multisource_nsources < 1) {
        return  GeneralErrorTemplates::kSimpleError.Generate("Number of sources size should be > 0");
    }


    return nullptr;
}

const EventMonConfig*  GetEventMonConfig() {
    return &config;
}


}