#include "request_handler_db_write.h"
#include "request.h"
#include "receiver_config.h"
#include "receiver_logger.h"

namespace asapo {

Error RequestHandlerDbWrite::ProcessRequest(Request* request) const {
    if (db_name_.empty()) {
        db_name_ = request->GetBeamtimeId();
    }

    if (Error err = ConnectToDbIfNeeded() ) {
        return err;
    }

    FileInfo file_info;
    file_info.name = request->GetFileName();
    file_info.size = request->GetDataSize();
    file_info.id = request->GetDataID();
    // todo: create flag ignore dups, allow dups for attempts to resend data
    auto err =  db_client__->Insert(file_info, true);
    if (!err) {
        log__->Debug(std::string{"insert record id "} + std::to_string(file_info.id) + " to " + kDBCollectionName + " in " +
                     db_name_ +
                     " at " + GetReceiverConfig()->broker_db_uri);
    }
    return err;
}

RequestHandlerDbWrite::RequestHandlerDbWrite(): log__{GetDefaultReceiverLogger()}  {
    DatabaseFactory factory;
    Error err;
    db_client__ = factory.Create(&err);
}

StatisticEntity RequestHandlerDbWrite::GetStatisticEntity() const {
    return StatisticEntity::kDatabase;
}

Error RequestHandlerDbWrite::ConnectToDbIfNeeded() const {
    if (!connected_to_db) {
        Error err = db_client__->Connect(GetReceiverConfig()->broker_db_uri, db_name_,
                                         kDBCollectionName);
        if (err) {
            return err;
        }
        connected_to_db = true;
    }
    return nullptr;
}


}