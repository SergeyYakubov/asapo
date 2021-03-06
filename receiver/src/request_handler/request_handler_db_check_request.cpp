#include "request_handler_db_check_request.h"

#include "asapo/database/database.h"
#include "asapo/database/db_error.h"
#include "asapo/logger/logger.h"
#include "request_handler_db.h"
#include "../receiver_logger.h"
#include "../request.h"

namespace asapo {

RequestHandlerDbCheckRequest::RequestHandlerDbCheckRequest(std::string collection_name_prefix) : RequestHandlerDb(
    std::move(
        collection_name_prefix)) {

}

Error RequestHandlerDbCheckRequest::GetRecordFromDb(const Request *request, MessageMeta *record) const {
    auto op_code = request->GetOpCode();
    auto id = request->GetDataID();
    auto col_name = collection_name_prefix_ + "_" + request->GetStream();
    Error err;
    if (op_code == Opcode::kOpcodeTransferData) {
        err = db_client__->GetById(col_name, id, record);
        if (!err) {
            log__->Debug(RequestLog("read message from database", request).
                Append("collection", col_name).Append("database", db_name_));
        }
        return err;
    } else {
        auto id_in_set = request->GetCustomData()[1];
        err = db_client__->GetDataSetById(col_name, id_in_set, id, record);
        if (!err) {
            log__->Debug(RequestLog("read dataset from database", request).
                Append("collection", col_name).Append("database", db_name_));
        }
        return err;
    }
}

bool RequestHandlerDbCheckRequest::SameRequestInRecord(const Request *request, const MessageMeta &record) const {
    std::string meta = request->GetMetaData();
    if (meta.size() == 0) {
        meta = "{}";  // so empty meta is stored in database
    }
    return request->GetDataSize() == record.size
        && request->GetFileName() == record.name
        && meta == record.metadata;
}

Error RequestHandlerDbCheckRequest::ProcessRequest(Request *request) const {
    if (auto err = RequestHandlerDb::ProcessRequest(request)) {
        return err;
    }

    MessageMeta record;
    auto err = GetRecordFromDb(request, &record);
    if (err) {
        return DBErrorToReceiverError(err == DBErrorTemplates::kNoRecord ? nullptr : std::move(err));
    }

    if (SameRequestInRecord(request, record)) {
        return ReceiverErrorTemplates::kWarningDuplicatedRequest.Generate();
    } else {
        return ReceiverErrorTemplates::kBadRequest.Generate("already have record with same id");
    }
}

}
