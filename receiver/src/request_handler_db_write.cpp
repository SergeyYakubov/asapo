#include "request_handler_db_write.h"
#include "request.h"
#include "receiver_config.h"

namespace hidra2 {

Error RequestHandlerDbWrite::ProcessRequest(const Request& request) const {
    if (Error err = ConnectToDbIfNeeded() ) {
        return err;
    }

    FileInfo file_info;
    file_info.name = request.GetFileName();
    file_info.size = request.GetDataSize();
    file_info.id = request.GetDataID();
    return db_client__->Insert(file_info, false);

}

RequestHandlerDbWrite::RequestHandlerDbWrite()  {
    DatabaseFactory factory;
    Error err;
    db_client__ = factory.Create(&err);
}

StatisticEntity RequestHandlerDbWrite::GetStatisticEntity() const {
    return StatisticEntity::kDatabase;
}

Error RequestHandlerDbWrite::ConnectToDbIfNeeded() const {
    if (!connected_to_db) {
        Error err = db_client__->Connect(GetReceiverConfig()->broker_db_uri, GetReceiverConfig()->broker_db_name,
                                         kDBCollectionName);
        if (err) {
            return err;
        }
        connected_to_db = true;
    }
    return nullptr;
}


}