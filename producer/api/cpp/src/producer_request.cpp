#include <asapo/asapo_producer.h>
#include "producer_request.h"
#include "asapo/common/internal/version.h"

namespace asapo {

bool ProducerRequest::DataFromFile() const {
    if (data != nullptr || original_filepath.empty() || !NeedSend()) {
        return false;
    }
    return true;
}

ProducerRequest::ProducerRequest(
                                 std::string source_credentials,
                                 GenericRequestHeader h,
                                 MessageData data,
                                 std::string metadata,
                                 std::string original_filepath,
                                 RequestCallback callback,
                                 bool manage_data_memory,
                                 uint64_t timeout_ms) : GenericRequest(std::move(h), timeout_ms),
                                                        source_credentials{std::move(source_credentials)},
                                                        metadata{std::move(metadata)},
                                                        data{std::move(data)},
                                                        original_filepath{std::move(original_filepath)},
                                                        callback{callback},
                                                        manage_data_memory{manage_data_memory} {

    if (kProducerProtocol.GetReceiverVersion().size() < kMaxVersionSize) {
        strcpy(header.api_version, kProducerProtocol.GetReceiverVersion().c_str());
    } else {
        strcpy(header.api_version, "v0.0");
    }
}

bool ProducerRequest::NeedSend() const {
    if (header.op_code == kOpcodeTransferData || header.op_code == kOpcodeTransferDatasetData) {
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
Error ProducerRequest::UpdateDataSizeFromFileIfNeeded(const IO* io) {
    if (!DataFromFile() || header.data_size > 0) {
        return nullptr;
    }

    Error err;
    auto message_meta = io->GetMessageMeta(original_filepath, &err);
    if (err) {
        return ProducerErrorTemplates::kLocalIOError.Generate(err->Explain());
    }
    header.data_size = message_meta.size;
    return nullptr;
}

}
