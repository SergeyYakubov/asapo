#include "request_handler_db_meta_write.h"
#include "request.h"
#include "receiver_config.h"
#include "receiver_logger.h"
#include "io/io_factory.h"


namespace asapo {

Error RequestHandlerDbMetaWrite::ProcessRequest(Request* request) const {
    if (Error err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    auto size = request->GetDataSize();
    auto meta = (uint8_t*)request->GetData();
    auto meta_id = request->GetDataID();

    auto err =  db_client__->Upsert(collection_name_prefix_, meta_id, meta, size);
    if (!err) {
        log__->Debug(std::string{"insert beamtime meta"} + " to " + collection_name_prefix_ + " in " +
                     db_name_ +
                     " at " + GetReceiverConfig()->database_uri);
    }
    return err;
}
RequestHandlerDbMetaWrite::RequestHandlerDbMetaWrite(std::string collection_name) : RequestHandlerDb(std::move(
                collection_name)) {

}

}
