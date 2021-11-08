#include "request_handler_db_meta_write.h"
#include "../request.h"
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

    auto err =  db_client__->InsertMeta(collection_name_prefix_, stream.empty() ? "bt" : "st_" + stream, meta, size, mode);
    if (!err) {
        if (stream.empty()) {
            log__->Debug(RequestLog("insert beamtime meta to database", request));

        } else {
            log__->Debug(RequestLog("insert stream meta to database", request));
        }

    }
    return DBErrorToReceiverError(std::move(err));
}
RequestHandlerDbMetaWrite::RequestHandlerDbMetaWrite(std::string collection_name) : RequestHandlerDb(std::move(
                collection_name)) {

}

}
