#ifndef HIDRA2_FOLDERDATABROKER_H
#define HIDRA2_FOLDERDATABROKER_H

#include "worker/data_broker.h"

#include <string>

namespace hidra2 {

class FolderDataBroker final: public hidra2::DataBroker {
  public:
    explicit FolderDataBroker(const std::string& source_name);
    WorkerErrorCode Connect() override;

    void set_io_(void* io);
  private:
    std::string source_name_;
    void* io_;
};

}

#endif //HIDRA2_FOLDERDATABROKER_H

