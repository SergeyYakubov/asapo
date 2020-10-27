#include "request_handler_db_stream_info.h"
#include "../receiver_config.h"

namespace asapo {

RequestHandlerDbStreamInfo::RequestHandlerDbStreamInfo(std::string collection_name_prefix)
    : RequestHandlerDb(std::move(collection_name_prefix)) {

}


Error RequestHandlerDbStreamInfo::ProcessRequest(Request* request) const {
    if (auto err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    auto col_name = collection_name_prefix_ + "_" + request->GetSubstream();
    StreamInfo info;
    auto err =  db_client__->GetStreamInfo(col_name, &info);
    if (!err) {
        log__->Debug(std::string{"get stream info from "} + col_name + " in " +
                     db_name_ + " at " + GetReceiverConfig()->database_uri);
        info.name = request->GetSubstream();
        request->SetResponseMessage(info.Json(true), ResponseMessageType::kInfo);
    }
    return err;
}

}