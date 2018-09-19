#include "eventmon_config.h"
#include "eventmon_config_factory.h"
#include "io/io_factory.h"
#include "json_parser/json_parser.h"

namespace asapo {

EventMonConfig config;

EventMonConfigFactory::EventMonConfigFactory() : io__{GenerateDefaultIO()} {

}

Error EventMonConfigFactory::ParseConfigFile(std::string file_name) {
    JsonFileParser parser(file_name, &io__);
    Error err = nullptr;
    (err = parser.GetString("AsapoEndpoint", &config.asapo_endpoint)) ||
    (err = parser.GetString("Tag", &config.tag)) ||
    (err = parser.GetString("BeamtimeID", &config.beamtime_id)) ||
    (err = parser.GetString("Mode", &config.mode_str)) ||
    (err = parser.GetUInt64("NThreads", &config.nthreads)) ||
    (err = parser.GetString("RootMonitoredFolder", &config.root_monitored_folder)) ||
    (err = parser.GetString("LogLevel", &config.log_level_str)) ||
    (err = parser.GetBool("RemoveAfterSend", &config.remove_after_send)) ||
    (err = parser.GetArrayString("MonitoredSubFolders", &config.monitored_subfolders)) ||
    (err = parser.GetArrayString("IgnoreExtentions", &config.ignored_extentions));

    return err;
}


Error EventMonConfigFactory::CheckConfig() {
    Error err;
    (err = CheckMode()) ||
    (err = CheckLogLevel()) ||
    (err = CheckNThreads());
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


Error EventMonConfigFactory::CheckNThreads() {
    if (config.nthreads == 0 || config.nthreads > kMaxProcessingThreads ) {
        return  TextError("NThreads should between 1 and " + std::to_string(kMaxProcessingThreads));
    }
    return nullptr;
}



const EventMonConfig*  GetEventMonConfig() {
    return &config;
}


}