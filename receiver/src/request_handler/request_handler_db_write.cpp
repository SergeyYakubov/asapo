#include "request_handler_db_write.h"

#include <chrono>

#include "asapo/io/io_factory.h"
#include "asapo/database/db_error.h"

#include "../request.h"
#include "../receiver_logger.h"
#include "../receiver_config.h"

namespace asapo {

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args ) {
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 );
}


Error RequestHandlerDbWrite::ProcessRequest(Request* request) const {
    if (request->WasAlreadyProcessed()) {
        return nullptr;
    }

    if (auto err = RequestHandlerDb::ProcessRequest(request) ) {
        return err;
    }

    auto err =  InsertRecordToDb(request);
    if (err == DBErrorTemplates::kDuplicateID) {
        return ProcessDuplicateRecordSituation(request);
    } else {
        return DBErrorToReceiverError(std::move(err));
    }
}

Error RequestHandlerDbWrite::ProcessDuplicateRecordSituation(Request* request) const {
    auto check_err = request->CheckForDuplicates();
    if (check_err == ReceiverErrorTemplates::kWarningDuplicatedRequest) {
        std::string warn_str = "ignoring duplicate record for id " + std::to_string(request->GetDataID());
        request->SetResponseMessage(warn_str, ResponseMessageType::kWarning);
        log__->Warning(RequestLog("ignoring duplicate record", request));
        return nullptr;
    }

    return check_err;
}


Error RequestHandlerDbWrite::InsertRecordToDb(const Request* request) const {
    auto message_meta = PrepareMessageMeta(request);

    auto op_code = request->GetOpCode();
    auto col_name = collection_name_prefix_ + "_" + request->GetStream();
    Error err;
    if (op_code == Opcode::kOpcodeTransferData) {
        uint64_t id_inserted{0};
        err =  db_client__->Insert(col_name, message_meta, false, &id_inserted);
        if (!err) {
            log__->Debug(RequestLog("insert record into database", request));
        }
    } else {
        message_meta.dataset_substream = request->GetCustomData()[1];
        auto dataset_size = request->GetCustomData()[2];
        err =  db_client__->InsertAsDatasetMessage(col_name, message_meta, dataset_size, false);
        if (!err) {
            log__->Debug(RequestLog("insert substream record into database", request));
        }
    }
    return err;
}

MessageMeta RequestHandlerDbWrite::PrepareMessageMeta(const Request* request) const {
    MessageMeta message_meta;
    message_meta.name = request->GetFileName();
    message_meta.size = request->GetDataSize();
    message_meta.id = request->GetDataID();
    message_meta.buf_id = request->GetSlotId();
    message_meta.stream = request->GetStream();
    message_meta.source = GetReceiverConfig()->dataserver.advertise_uri;
    message_meta.metadata = request->GetMetaData();
    message_meta.timestamp = std::chrono::system_clock::now();
    return message_meta;
}

RequestHandlerDbWrite::RequestHandlerDbWrite(std::string collection_name_prefix) : RequestHandlerDb(std::move(
                collection_name_prefix)) {

}


}
