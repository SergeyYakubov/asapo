#include "request_handler_db.h"
#include "receiver_config.h"
#include "receiver_logger.h"
#include "request.h"

namespace asapo {

Error RequestHandlerDb::ProcessRequest(Request* request) const {
    if (db_name_.empty()) {
        db_name_ = request->GetBeamtimeId();
    }

    return ConnectToDbIfNeeded();
}

RequestHandlerDb::RequestHandlerDb(std::string collection_name): log__{GetDefaultReceiverLogger()},
    collection_name_{std::move(collection_name)} {
    DatabaseFactory factory;
    Error err;
    db_client__ = factory.Create(&err);
}

StatisticEntity RequestHandlerDb::GetStatisticEntity() const {
    return StatisticEntity::kDatabase;
}

Error RequestHandlerDb::ConnectToDbIfNeeded() const {
    if (!connected_to_db) {
        Error err = db_client__->Connect(GetReceiverConfig()->broker_db_uri, db_name_,
                                         collection_name_);
        if (err) {
            return err;
        }
        connected_to_db = true;
    }
    return nullptr;
}

}