#include "request_handler_filesystem.h"

#include <cstdint>

#include "asapo/producer/producer_error.h"
#include "producer_logger.h"
#include "asapo/io/io_factory.h"

#include "producer_request.h"


namespace asapo {

RequestHandlerFilesystem::RequestHandlerFilesystem(std::string destination_folder, uint64_t thread_id):
    io__{GenerateDefaultIO()}, log__{GetDefaultProducerLogger()}, destination_folder_{std::move(destination_folder)},
    thread_id_{thread_id} {
}

bool RequestHandlerFilesystem::ProcessRequestUnlocked(GenericRequest* request, bool* retry) {
    auto producer_request = static_cast<ProducerRequest*>(request);
    Error err;
    if (producer_request->DataFromFile()) {
        producer_request->data = io__->GetDataFromFile(producer_request->original_filepath, &producer_request->header.data_size,
                                                       &err);
        if (err) {
            producer_request->data = nullptr;
            *retry = true;
            return false;
        }
    }

    err = io__->WriteDataToFile(destination_folder_, request->header.message, (uint8_t*)producer_request->data.get(),
                                (size_t)request->header.data_size, true, true);
    if (producer_request->callback) {
        producer_request->callback(RequestCallbackPayload{request->header, std::move(producer_request->data),""}, std::move(err));
    }
    *retry = false;
    return true;
}

void RequestHandlerFilesystem::ProcessRequestTimeoutUnlocked(GenericRequest* request) {
    log__->Error("request timeout, id:" + std::to_string(request->header.data_id) + " to " + request->header.stream +
                 " stream");
}

}
