#include "foldermon_config.h"
#include "foldermon_config_factory.h"
#include "io/io_factory.h"
#include "json_parser/json_parser.h"

namespace asapo {

FolderMonConfig config;

FolderMonConfigFactory::FolderMonConfigFactory() : io__{GenerateDefaultIO()} {

}

Error FolderMonConfigFactory::ParseConfigFile(std::string file_name) {
    JsonFileParser parser(file_name, &io__);
    Error err = nullptr;
    (err = parser.GetString("AsapoEndpoint", &config.asapo_endpoint)) ||
    (err = parser.GetString("Tag", &config.tag)) ||
    (err = parser.GetString("BeamtimeID", &config.beamtime_id)) ||
    (err = parser.GetString("Mode", &config.mode_str)) ||
    (err = parser.GetUInt64("NThreads", &config.nthreads)) ||
    (err = parser.GetString("LogLevel", &config.log_level_str));
    return err;
}


Error FolderMonConfigFactory::CheckConfig() {
    Error err;
    (err = CheckMode()) ||
    (err = CheckLogLevel()) ||
    (err = CheckNThreads());
    return err;
}


Error FolderMonConfigFactory::SetConfigFromFile(std::string file_name) {
    auto  err = ParseConfigFile(file_name);
    if (err) {
        return err;
    }

    return CheckConfig();
}

Error FolderMonConfigFactory::CheckMode() {
    if (config.mode_str == "tcp") {
        config.mode = RequestHandlerType::kTcp;
    } else if (config.mode_str == "filesystem") {
        config.mode = RequestHandlerType::kFilesystem;
    } else {
        return  TextError("wrong producer mode: " + config.mode_str);
    }
    return nullptr;
}

Error FolderMonConfigFactory::CheckLogLevel() {
    Error err;
    config.log_level = StringToLogLevel(config.log_level_str, &err);
    return err;
}


Error FolderMonConfigFactory::CheckNThreads() {
    if (config.nthreads == 0 || config.nthreads > kMaxProcessingThreads ) {
        return  TextError("NThreads should between 1 and " + std::to_string(kMaxProcessingThreads));
    }
    return nullptr;
}



const FolderMonConfig*  GetFolderMonConfig() {
    return &config;
}


}