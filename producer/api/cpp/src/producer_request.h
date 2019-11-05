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
    ~ProducerRequest();
    ProducerRequest(std::string source_credentials, GenericRequestHeader header, FileData data,
                    std::string metadata,
                    std::string original_filepath,
                    RequestCallback callback,
                    bool manage_data_memory);
    std::string source_credentials;
    std::string metadata;
    FileData data;
    std::string original_filepath;
    RequestCallback callback;
    bool manage_data_memory;
    Error ReadDataFromFileIfNeeded(const IO* io);
    bool NeedSendData() const;
    bool NeedSendMetaData() const;
};

}

#endif //ASAPO_PRODUCER_REQUEST_H
