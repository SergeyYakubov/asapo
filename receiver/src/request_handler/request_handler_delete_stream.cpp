#include "request_handler_delete_stream.h"
#include "../receiver_config.h"
#include <asapo/database/db_error.h>

namespace asapo {

RequestHandlerDeleteStream::RequestHandlerDeleteStream(std::string collection_name_prefix) : RequestHandlerDb(
    std::move(collection_name_prefix)) {
}

Error RequestHandlerDeleteStream::ProcessRequest(Request* request) const {
    if (auto err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    DeleteStreamOptions options{};
    uint64_t flag = request->GetCustomData()[0];
    options.Decode(flag);
    auto col_name = collection_name_prefix_ + "_" + request->GetStream();
    auto err =  db_client__->DeleteStream(col_name);

    bool no_error = err == nullptr;
    if (err == DBErrorTemplates::kNoRecord && !options.error_on_not_exist) {
        no_error = true;
    }

    if (no_error) {
        log__->Debug(std::string{"deleted stream in "} + col_name + " in " +
            db_name_ + " at " + GetReceiverConfig()->database_uri);
        return nullptr;
    }

    return err;
}


}