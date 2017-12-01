#ifndef HIDRA2_FOLDERDATABROKER_H
#define HIDRA2_FOLDERDATABROKER_H

#include "worker/data_source.h"

#include <string>

namespace hidra2 {

class FolderDataBroker : public hidra2::DataBroker {
  public:
    FolderDataBroker(std::string source_name);
    int Connect() override;
  private:
    std::string source_name_;
};

}

#endif //HIDRA2_FOLDERDATABROKER_H
