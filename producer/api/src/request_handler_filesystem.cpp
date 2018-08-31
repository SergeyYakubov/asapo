#include "producer/producer_error.h"
#include "request_handler_filesystem.h"
#include "producer_logger.h"
#include "io/io_factory.h"

#include <cstdint>

namespace asapo {

RequestHandlerFilesystem::RequestHandlerFilesystem(std::string destination_folder, uint64_t thread_id):
    io__{GenerateDefaultIO()}, log__{GetDefaultProducerLogger()}, destination_folder_{std::move(destination_folder)},
    thread_id_{thread_id} {

}

Error RequestHandlerFilesystem::ProcessRequestUnlocked(Request* request) {
    auto err = request->ReadDataFromFileIfNeeded(io__.get());
    if (err) {
        return err;
    }

    err = io__->WriteDataToFile(destination_folder_, request->header.message, (uint8_t*)request->data.get(),
                                request->header.data_size, true);
    if (request->callback) {
        request->callback(request->header, std::move(err));
    }
    return nullptr;
}


}
