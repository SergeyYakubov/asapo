#include "request_handler_db_delete_stream.h"
#include <asapo/database/db_error.h>

#include "../receiver_logger.h"

namespace asapo {

RequestHandlerDbDeleteStream::RequestHandlerDbDeleteStream(std::string collection_name_prefix) : RequestHandlerDb(
        std::move(collection_name_prefix)) {
}

Error RequestHandlerDbDeleteStream::ProcessRequest(Request* request) const {
    if (auto err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    DeleteStreamOptions options{};
    uint64_t flag = request->GetCustomData()[0];
    options.Decode(flag);
    auto stream_name = request->GetStream();

    if (!options.delete_meta) {
        log__->Debug(RequestLog("skipped deleting stream meta", request));
        return nullptr;
    }
    auto err =  db_client__->DeleteStream(stream_name);

    bool no_error = err == nullptr;
    if (err == DBErrorTemplates::kNoRecord && !options.error_on_not_exist) {
        no_error = true;
    }

    if (no_error) {
        log__->Debug(RequestLog("deleted stream meta", request));
        return nullptr;
    }

    return DBErrorToReceiverError(std::move(err));
}


}