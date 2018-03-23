#include "request_handler_file_write.h"
#include "system_wrappers/system_io.h"

namespace hidra2 {

Error RequestHandlerFileWrite::ProcessRequest(const Request& request) const {
    return nullptr;
}

RequestHandlerFileWrite::RequestHandlerFileWrite() : io__{new SystemIO} {

}

}
