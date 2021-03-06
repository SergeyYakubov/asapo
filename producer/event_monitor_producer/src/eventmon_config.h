#ifndef ASAPO_EventMon_CONFIG_H
#define ASAPO_EventMon_CONFIG_H

#include "asapo/io/io.h"
#include "asapo/common/error.h"
#include "asapo/logger/logger.h"
#include "asapo/asapo_producer.h"
#include "eventmon_config_factory.h"

namespace asapo {

enum class DatasetMode {
    kNone,
    kBatch,
    kMultiSource
};

struct EventMonConfig {
    std::string asapo_endpoint;
    LogLevel log_level = LogLevel::Info;
    std::string tag;
    uint64_t nthreads = 1;
    std::string beamtime_id;
    RequestHandlerType mode = RequestHandlerType::kTcp;
    std::string root_monitored_folder;
    std::vector<std::string> monitored_subfolders;
    std::vector<std::string> ignored_extensions;
    std::vector<std::string> whitelisted_extensions;
    bool remove_after_send = false;
    DatasetMode dataset_mode = DatasetMode::kNone;
    uint64_t dataset_batch_size = 1;
    uint64_t dataset_multisource_nsources = 1;
    uint64_t dataset_multisource_sourceid = 1;
    std::string data_source;
  private:
    std::string log_level_str;
    std::string mode_str;
    friend EventMonConfigFactory;
};

const EventMonConfig* GetEventMonConfig();

}


#endif //ASAPO_EventMon_CONFIG_H
