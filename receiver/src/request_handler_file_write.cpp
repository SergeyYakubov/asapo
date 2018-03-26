#include "request_handler_file_write.h"
#include "system_wrappers/system_io.h"
#include "request.h"
namespace hidra2 {

Error RequestHandlerFileWrite::ProcessRequest(const Request& request) const {
    auto fsize = request.GetDataSize();
    if (fsize <= 0 || fsize > kMaxFileSize) {
        return ReceiverErrorTemplates::kBadRequest.Generate();
    }

    const FileData& data = request.GetData();

    auto fname = request.GetFileName();

    return io__->WriteDataToFile("files/" + fname, data, fsize);

}

RequestHandlerFileWrite::RequestHandlerFileWrite() : io__{new SystemIO} {

}

}
