#include "producer_request.h"

namespace asapo {

Error ProducerRequest::ReadDataFromFileIfNeeded(const IO* io) {
    if (data != nullptr || original_filepath.empty()) {
        return nullptr;
    }
    Error err;
    data = io->GetDataFromFile(original_filepath, &header.data_size, &err);
    return err;
}

ProducerRequest::ProducerRequest(std::string beamtime_id,
                                 GenericRequestHeader h,
                                 FileData data,
                                 std::string metadata,
                                 std::string original_filepath,
                                 RequestCallback callback) : GenericRequest(std::move(h)),
    beamtime_id{std::move(beamtime_id)},
    metadata{std::move(metadata)},
    data{std::move(data)},
    original_filepath{std::move(original_filepath)},
    callback{callback} {
}

}