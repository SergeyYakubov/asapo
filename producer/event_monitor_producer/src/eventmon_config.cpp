#include "eventmon_config.h"
#include "eventmon_config_factory.h"
#include "io/io_factory.h"
#include "json_parser/json_parser.h"

namespace asapo {

EventMonConfig config;

EventMonConfigFactory::EventMonConfigFactory() : io__{GenerateDefaultIO()} {

}

Error SubsetModeToEnum(const std::string& mode_str, SubSetMode* mode) {
    if (mode_str == "batch") {
        *mode = SubSetMode::kBatch;
        return nullptr;
    }

    if (mode_str == "none") {
        *mode = SubSetMode::kNone;
        return nullptr;
    }

    if (mode_str == "multisource") {
        *mode = SubSetMode::kMultiSource;
        return nullptr;
    }


    return TextError("Wrone subset mode:" + mode_str);
}

Error EventMonConfigFactory::ParseConfigFile(std::string file_name) {
    JsonFileParser parser(file_name, &io__);
    Error err = nullptr;
    std::string subset_mode;

    (err = parser.GetString("AsapoEndpoint", &config.asapo_endpoint)) ||
    (err = parser.GetString("Tag", &config.tag)) ||
    (err = parser.GetString("BeamtimeID", &config.beamtime_id)) ||
    (err = parser.GetString("Stream", &config.stream)) ||
    (err = parser.GetString("Mode", &config.mode_str)) ||
    (err = parser.GetUInt64("NThreads", &config.nthreads)) ||
    (err = parser.GetString("RootMonitoredFolder", &config.root_monitored_folder)) ||
    (err = parser.GetString("LogLevel", &config.log_level_str)) ||
    (err = parser.GetBool("RemoveAfterSend", &config.remove_after_send)) ||
    (err = parser.GetArrayString("MonitoredSubFolders", &config.monitored_subfolders)) ||
    (err = parser.GetArrayString("IgnoreExtensions", &config.ignored_extensions)) ||
    (err = parser.GetArrayString("WhitelistExtensions", &config.whitelisted_extensions)) ||
    (err = parser.Embedded("Subset").GetString("Mode", &subset_mode)) ||
    (err = SubsetModeToEnum(subset_mode, &config.subset_mode));
    if (err) {
        return err;
    }

    if (config.subset_mode == SubSetMode::kBatch) {
        err = parser.Embedded("Subset").GetUInt64("BatchSize", &config.subset_batch_size);
    }

    if (config.subset_mode == SubSetMode::kMultiSource) {
        err = parser.Embedded("Subset").GetUInt64("NSources", &config.subset_multisource_nsources);
        err = parser.Embedded("Subset").GetUInt64("SourceId", &config.subset_multisource_sourceid);
    }


    return err;
}


Error EventMonConfigFactory::CheckConfig() {
    Error err;
    (err = CheckMode()) ||
    (err = CheckLogLevel()) ||
    (err = CheckNThreads()) ||
    (err = CheckBlackWhiteLists()) ||
    (err = CheckSubsets());

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
        return  TextError("wrong producer mode: " + config.mode_str);
    }
    return nullptr;
}

Error EventMonConfigFactory::CheckLogLevel() {
    Error err;
    config.log_level = StringToLogLevel(config.log_level_str, &err);
    return err;
}

Error EventMonConfigFactory::CheckBlackWhiteLists() {
    if (config.whitelisted_extensions.size() && config.ignored_extensions.size() ) {
        return  TextError("only one of IgnoreExtensions/WhitelistExtensions can be set");
    }
    return nullptr;
}


Error EventMonConfigFactory::CheckNThreads() {
    if (config.nthreads == 0 || config.nthreads > kMaxProcessingThreads ) {
        return  TextError("NThreads should between 1 and " + std::to_string(kMaxProcessingThreads));
    }
    return nullptr;
}

Error EventMonConfigFactory::CheckSubsets() {
    if (config.subset_mode == SubSetMode::kBatch && config.subset_batch_size < 1) {
        return  TextError("Batch size should > 0");
    }


    if (config.subset_mode == SubSetMode::kMultiSource && config.subset_multisource_nsources < 1) {
        return  TextError("Number of sources size should be > 0");
    }


    return nullptr;
}

const EventMonConfig*  GetEventMonConfig() {
    return &config;
}


}