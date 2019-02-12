#ifndef ASAPO_MOCK_RECEIVER_CONFIG_H
#define ASAPO_MOCK_RECEIVER_CONFIG_H

#include "../src/receiver_config.h"


namespace asapo {

Error SetReceiverConfig (const ReceiverConfig& config, std::string error_field);

}


#endif
