#ifndef HIDRA2_SERVER_DATA_BROKER_H
#define HIDRA2_SERVER_DATA_BROKER_H

#include "worker/data_broker.h"

namespace hidra2 {

class ServerDataBroker final : public hidra2::DataBroker {
  public:
    explicit ServerDataBroker(const std::string& source_name);
    WorkerErrorCode Connect() override;
    WorkerErrorCode GetNext(FileInfo* info, FileData* data) override;
};

}

#endif //HIDRA2_SERVER_DATA_BROKER_H
