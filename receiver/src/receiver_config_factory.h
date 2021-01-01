#ifndef ASAPO_RECEIVER_CONFIG_FACTORY__H
#define ASAPO_RECEIVER_CONFIG_FACTORY__H

#include "asapo/io/io.h"
#include "asapo/common/error.h"

namespace asapo {

class ReceiverConfigFactory {
  public:
    ReceiverConfigFactory();
    Error SetConfig(std::string file_name);
  public:
    std::unique_ptr<IO> io__;
};

}


#endif //ASAPO_RECEIVER_CONFIG_FACTORY__H
