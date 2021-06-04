#include "request_handler_db_last_stream.h"
#include "../receiver_config.h"


namespace asapo {

RequestHandlerDbLastStream::RequestHandlerDbLastStream(std::string collection_name_prefix)
    : RequestHandlerDb(std::move(collection_name_prefix)) {

}


Error RequestHandlerDbLastStream::ProcessRequest(Request* request) const {
    if (auto err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    StreamInfo info;
    auto err =  db_client__->GetLastStream(&info);
    if (!err) {
        log__->Debug(std::string{"get last stream "} + " in " +
            db_name_ + " at " + GetReceiverConfig()->database_uri);
        request->SetResponseMessage(info.Json(), ResponseMessageType::kInfo);
    }
    return DBErrorToReceiverError(err);
}

}