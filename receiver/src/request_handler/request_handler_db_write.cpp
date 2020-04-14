#include "request_handler_db_write.h"
#include "../request.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "io/io_factory.h"
#include "database/db_error.h"


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
        return err;
    }
}

Error RequestHandlerDbWrite::ProcessDuplicateRecordSituation(Request* request) const {
    auto check_err = request->CheckForDuplicates();
    if (check_err == ReceiverErrorTemplates::kWarningDuplicatedRequest) {
        std::string warn_str = "ignoring duplicate record for id " + std::to_string(request->GetDataID());
        request->SetWarningMessage(warn_str);
        log__->Warning(warn_str);
        return nullptr;
    }

    return check_err;
}


Error RequestHandlerDbWrite::InsertRecordToDb(const Request* request) const {
    auto file_info = PrepareFileInfo(request);

    auto op_code = request->GetOpCode();
    auto col_name = collection_name_prefix_ + "_" + request->GetSubstream();
    Error err;
    if (op_code == Opcode::kOpcodeTransferData) {
        err =  db_client__->Insert(col_name, file_info, false);
        if (!err) {
            log__->Debug(std::string{"insert record id "} + std::to_string(file_info.id) + " to " + col_name + " in " +
                         db_name_ +
                         " at " + GetReceiverConfig()->database_uri);
        }
    } else {
        auto subset_id = request->GetCustomData()[1];
        auto subset_size = request->GetCustomData()[2];
        err =  db_client__->InsertAsSubset(col_name, file_info, subset_id, subset_size, false);
        if (!err) {
            log__->Debug(std::string{"insert record as subset id "} + std::to_string(subset_id) + ", id: " +
                         std::to_string(file_info.id) + " to " + col_name + " in " +
                         db_name_ +
                         " at " + GetReceiverConfig()->database_uri);
        }
    }
    return err;
}

FileInfo RequestHandlerDbWrite::PrepareFileInfo(const Request* request) const {
    FileInfo file_info;
    file_info.name = request->GetFileName();
    file_info.size = request->GetDataSize();
    file_info.id = request->GetDataID();
    file_info.buf_id = request->GetSlotId();
    file_info.source = GetReceiverConfig()->dataserver.advertise_uri;
    file_info.metadata = request->GetMetaData();
    return file_info;
}

RequestHandlerDbWrite::RequestHandlerDbWrite(std::string collection_name_prefix) : RequestHandlerDb(std::move(
                collection_name_prefix)) {

}


}
