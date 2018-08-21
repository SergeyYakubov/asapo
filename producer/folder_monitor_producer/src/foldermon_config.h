#ifndef ASAPO_FolderMon_CONFIG_H
#define ASAPO_FolderMon_CONFIG_H

#include "io/io.h"
#include "common/error.h"
#include "logger/logger.h"
#include "producer/common.h"
#include "foldermon_config_factory.h"

namespace asapo {

struct FolderMonConfig {
    std::string asapo_endpoint;
    LogLevel log_level = LogLevel::Info;
    std::string tag;
    uint64_t nthreads = 1;
    std::string beamtime_id;
    RequestHandlerType mode = RequestHandlerType::kTcp;
  private:
    std::string log_level_str;
    std::string mode_str;
    friend FolderMonConfigFactory;
};

const FolderMonConfig* GetFolderMonConfig();

}


#endif //ASAPO_FolderMon_CONFIG_H
