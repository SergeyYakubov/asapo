#include "request_handler_file_write.h"
#include "io/io_factory.h"
#include "request.h"
namespace hidra2 {

Error RequestHandlerFileWrite::ProcessRequest(const Request& request) const {
    auto fsize = request.GetDataSize();
    if (fsize <= 0 || fsize > kMaxFileSize) {
        return ReceiverErrorTemplates::kBadRequest.Generate();
    }

    const FileData& data = request.GetData();

    auto fname = request.GetFileName();
//TODO: folder to write in config file
    return io__->WriteDataToFile("files/" + fname, data, fsize);

}

RequestHandlerFileWrite::RequestHandlerFileWrite() : io__{GenerateDefaultIO()} {

}

StatisticEntity RequestHandlerFileWrite::GetStatisticEntity() const {
    return StatisticEntity::kDisk;
}


}
