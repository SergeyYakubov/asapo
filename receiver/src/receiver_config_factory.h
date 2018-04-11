#ifndef HIDRA2_RECEIVER_CONFIG_FACTORY__H
#define HIDRA2_RECEIVER_CONFIG_FACTORY__H

#include "io/io.h"
#include "common/error.h"

namespace hidra2 {

class ReceiverConfigFactory {
  public:
    ReceiverConfigFactory();
    Error SetConfigFromFile(std::string file_name);
  public:
    std::unique_ptr<IO> io__;
};

}


#endif //HIDRA2_RECEIVER_CONFIG_FACTORY__H
