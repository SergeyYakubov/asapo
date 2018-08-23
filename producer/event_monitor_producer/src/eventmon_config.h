#ifndef ASAPO_EventMon_CONFIG_H
#define ASAPO_EventMon_CONFIG_H

#include "io/io.h"
#include "common/error.h"
#include "logger/logger.h"
#include "producer/common.h"
#include "eventmon_config_factory.h"

namespace asapo {

struct EventMonConfig {
    std::string asapo_endpoint;
    LogLevel log_level = LogLevel::Info;
    std::string tag;
    uint64_t nthreads = 1;
    std::string beamtime_id;
    RequestHandlerType mode = RequestHandlerType::kTcp;
  private:
    std::string log_level_str;
    std::string mode_str;
    friend EventMonConfigFactory;
};

const EventMonConfig* GetEventMonConfig();

}


#endif //ASAPO_EventMon_CONFIG_H
