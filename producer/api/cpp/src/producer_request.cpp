#include "producer_request.h"

namespace asapo {

Error ProducerRequest::ReadDataFromFileIfNeeded(const IO* io) {
    if (data != nullptr || original_filepath.empty() || !NeedSendData()) {
        return nullptr;
    }
    Error err;
    data = io->GetDataFromFile(original_filepath, &header.data_size, &err);
    return err;
}

ProducerRequest::ProducerRequest(std::string source_credentials,
                                 GenericRequestHeader h,
                                 FileData data,
                                 std::string metadata,
                                 std::string original_filepath,
                                 RequestCallback callback,
                                 bool manage_data_memory) : GenericRequest(std::move(h)),
    source_credentials{std::move(source_credentials)},
    metadata{std::move(metadata)},
    data{std::move(data)},
    original_filepath{std::move(original_filepath)},
    callback{callback},
    manage_data_memory{manage_data_memory} {
}

bool ProducerRequest::NeedSendData() const {
    if (header.op_code == kOpcodeTransferData || header.op_code == kOpcodeTransferSubsetData) {
        return header.custom_data[kPosIngestMode] & IngestModeFlags::kTransferData;
    }
    return true;
}

bool ProducerRequest::NeedSendMetaData() const {
    return header.meta_size > 0;
}


ProducerRequest::~ProducerRequest() {
    if (!manage_data_memory && data != nullptr) {
        data.release();
    }
}

}