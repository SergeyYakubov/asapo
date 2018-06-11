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

Error RequestHandlerFilesystem::ProcessRequestUnlocked(const Request* request) {
    std::string fullpath = destination_folder_ + "/" + request->header.message + ".bin";
    auto err = io__->WriteDataToFile(fullpath, (uint8_t*)request->data, request->header.data_size);
    if (request->callback) {
        request->callback(request->header, std::move(err));
    }
    return nullptr;
}


}
