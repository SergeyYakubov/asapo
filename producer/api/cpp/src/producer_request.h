#ifndef ASAPO_PRODUCER_REQUEST_H
#define ASAPO_PRODUCER_REQUEST_H

#include "asapo/common/networking.h"
#include "asapo/producer/common.h"
#include "asapo/common/data_structs.h"
#include "asapo/io/io.h"
#include "asapo/request/request.h"

namespace asapo {

class ProducerRequest : public GenericRequest {
  public:
    ~ProducerRequest();
    ProducerRequest(std::string source_credentials, GenericRequestHeader header, MessageData data,
                    std::string metadata,
                    std::string original_filepath,
                    RequestCallback callback,
                    bool manage_data_memory,
                    uint64_t timeout_ms);
    virtual bool ContainsData() override {
        return !DataFromFile();
    };
    std::string source_credentials;
    std::string metadata;
    MessageData data;
    std::string original_filepath;
    RequestCallback callback;
    bool manage_data_memory;
    bool DataFromFile() const;
    bool NeedSend() const;
    bool NeedSendMetaData() const;
    Error UpdateDataSizeFromFileIfNeeded(const IO* io);

};

}

#endif //ASAPO_PRODUCER_REQUEST_H
