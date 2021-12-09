#include "request_handler_db_stream_info.h"
#include "../receiver_logger.h"

namespace asapo {

RequestHandlerDbStreamInfo::RequestHandlerDbStreamInfo(std::string collection_name_prefix)
    : RequestHandlerDb(std::move(collection_name_prefix)) {

}


Error RequestHandlerDbStreamInfo::ProcessRequest(Request* request) const {
    if (auto err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    auto col_name = collection_name_prefix_ + "_" + request->GetStream();
    StreamInfo info;
    auto err =  db_client__->GetStreamInfo(col_name, &info);
    if (!err) {
        log__->Debug(RequestLog("get stream info from database", request));
        info.name = request->GetStream();
        request->SetResponseMessage(info.Json(), ResponseMessageType::kInfo);
    }
    return DBErrorToReceiverError(std::move(err));
}

}