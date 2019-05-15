#ifndef ASAPO_PRODUCER_REQUEST_H
#define ASAPO_PRODUCER_REQUEST_H

#include "common/networking.h"
#include "producer/common.h"
#include "common/data_structs.h"
#include "io/io.h"
#include "request/request.h"

namespace asapo {

class ProducerRequest : public GenericRequest {
  public:
    ProducerRequest(std::string beamtime_id, GenericRequestHeader header, FileData data, std::string original_filepath,
                    RequestCallback callback);
    std::string beamtime_id;
    FileData data;
    std::string original_filepath;
    RequestCallback callback;
    Error ReadDataFromFileIfNeeded(const IO* io);
};

}

#endif //ASAPO_PRODUCER_REQUEST_H
