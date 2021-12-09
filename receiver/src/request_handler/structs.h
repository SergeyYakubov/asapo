#ifndef ASAPO_RECEIVER_SRC_REQUEST_HANDLER_STRUCTS_H_
#define ASAPO_RECEIVER_SRC_REQUEST_HANDLER_STRUCTS_H_

#include <chrono>

#include "asapo/common/data_structs.h"

namespace asapo {

struct AuthorizationData {
    std::string producer_instance_id;
    std::string pipeline_step_id;
    std::string beamtime_id;
    std::string data_source;
    std::string beamline;
    std::string offline_path;
    std::string online_path;
    SourceType source_type;
    std::chrono::system_clock::time_point last_update;
    std::string source_credentials;
};

}

#endif //ASAPO_RECEIVER_SRC_REQUEST_HANDLER_STRUCTS_H_
