#include "request_handler_db_write.h"

#include <chrono>


#include "../request.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "asapo/io/io_factory.h"
#include "asapo/database/db_error.h"


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
        return DBErrorToReceiverError(err);
    }
}

Error RequestHandlerDbWrite::ProcessDuplicateRecordSituation(Request* request) const {
    auto check_err = request->CheckForDuplicates();
    if (check_err == ReceiverErrorTemplates::kWarningDuplicatedRequest) {
        std::string warn_str = "ignoring duplicate record for id " + std::to_string(request->GetDataID());
        request->SetResponseMessage(warn_str, ResponseMessageType::kWarning);
        log__->Warning(warn_str);
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
        err =  db_client__->Insert(col_name, message_meta, false);
        if (!err) {
            log__->Debug(std::string{"insert record id "} + std::to_string(message_meta.id) + " to " + col_name + " in " +
                         db_name_ +
                         " at " + GetReceiverConfig()->database_uri);
        }
    } else {
        message_meta.dataset_substream = request->GetCustomData()[1];
        auto dataset_size = request->GetCustomData()[2];
        err =  db_client__->InsertAsDatasetMessage(col_name, message_meta, dataset_size, false);
        if (!err) {
            log__->Debug(std::string{"insert record to substream "} + std::to_string(message_meta.dataset_substream) + ", id: " +
                         std::to_string(message_meta.id) + " to " + col_name + " in " +
                         db_name_ +
                         " at " + GetReceiverConfig()->database_uri);
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
    message_meta.source = GetReceiverConfig()->dataserver.advertise_uri;
    message_meta.metadata = request->GetMetaData();
    message_meta.timestamp = std::chrono::system_clock::now();
    return message_meta;
}

RequestHandlerDbWrite::RequestHandlerDbWrite(std::string collection_name_prefix) : RequestHandlerDb(std::move(
                collection_name_prefix)) {

}


}
