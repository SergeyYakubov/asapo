#ifndef HIDRA2_FOLDERDATABROKER_H
#define HIDRA2_FOLDERDATABROKER_H

#include "worker/data_broker.h"

#include <string>

namespace hidra2 {

class FolderDataBroker final: public hidra2::DataBroker {
  public:
    FolderDataBroker(std::string source_name);
    int connect() override;
 private:
    std::string source_name_;
};

}

#endif //HIDRA2_FOLDERDATABROKER_H
