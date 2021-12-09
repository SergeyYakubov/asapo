#include "request_handler_db_get_meta.h"
#include "../receiver_logger.h"
#include <asapo/database/db_error.h>

namespace asapo {

RequestHandlerDbGetMeta::RequestHandlerDbGetMeta(std::string collection_name_prefix) : RequestHandlerDb(
        std::move(collection_name_prefix)) {
}

Error RequestHandlerDbGetMeta::ProcessRequest(Request* request) const {
    if (auto err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    auto stream_name = request->GetStream();

    std::string metaid = stream_name.empty() ? "bt" : "st_" + stream_name;
    std::string meta;
    auto err =  db_client__->GetMetaFromDb(kDBMetaCollectionName, metaid, &meta);

    if (err == nullptr || err == DBErrorTemplates::kNoRecord) {
        log__->Debug(RequestLog("retrieved meta from database", request));
        request->SetResponseMessage(meta, ResponseMessageType::kInfo);
        return nullptr;
    }

    return DBErrorToReceiverError(std::move(err));
}


}