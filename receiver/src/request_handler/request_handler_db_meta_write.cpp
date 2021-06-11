#include "request_handler_db_meta_write.h"
#include "../request.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "asapo/io/io_factory.h"
#include "asapo/common/internal/version.h"

namespace asapo {

Error RequestHandlerDbMetaWrite::ProcessRequest(Request* request) const {
    if (Error err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    auto size = request->GetDataSize();
    auto meta = (uint8_t*)request->GetData();
    auto api_version = VersionToNumber(request->GetApiVersion());

    std::string stream;
    MetaIngestMode mode;
    if (api_version < 3) {
        // old approach, deprecates 01.07.2022
        mode.op = MetaIngestOp::kReplace;
        mode.upsert = true;
    } else {
        stream = request->GetStream();
        mode.Decode(request->GetCustomData()[kPosMetaIngestMode]);
    }

    auto err =  db_client__->Insert(collection_name_prefix_, stream.empty() ? "bt" : "st_" + stream, meta, size, mode);
    if (!err) {
        if (stream.empty()) {
            log__->Debug(std::string{"insert beamtime meta"} + " to " + collection_name_prefix_ + " in " +
                         db_name_ +
                         " at " + GetReceiverConfig()->database_uri);
        } else {
            log__->Debug(std::string{"insert stream meta for "} +stream + " to " + collection_name_prefix_ + " in " +
                         db_name_ +
                         " at " + GetReceiverConfig()->database_uri);
        }

    }
    return DBErrorToReceiverError(err);
}
RequestHandlerDbMetaWrite::RequestHandlerDbMetaWrite(std::string collection_name) : RequestHandlerDb(std::move(
                collection_name)) {

}

}
