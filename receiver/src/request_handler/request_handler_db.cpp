#include "request_handler_db.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"

namespace asapo {

Error RequestHandlerDb::ProcessRequest(Request* request) const {
    if (db_name_.empty()) {
        db_name_ = request->GetBeamtimeId();
        auto data_source = request->GetDataSource();
        db_name_ += "_" + data_source;
    }


    return ConnectToDbIfNeeded();
}

RequestHandlerDb::RequestHandlerDb(std::string collection_name_prefix): log__{GetDefaultReceiverLogger()},
    http_client__{DefaultHttpClient()},
    collection_name_prefix_{std::move(collection_name_prefix)} {
    DatabaseFactory factory;
    Error err;
    db_client__ = factory.Create(&err);
}

StatisticEntity RequestHandlerDb::GetStatisticEntity() const {
    return StatisticEntity::kDatabase;
}


Error RequestHandlerDb::GetDatabaseServerUri(std::string* uri) const {
    if (GetReceiverConfig()->database_uri != "auto") {
        *uri = GetReceiverConfig()->database_uri;
        return nullptr;
    }

    HttpCode code;
    Error http_err;
    *uri = http_client__->Get(GetReceiverConfig()->discovery_server + "/asapo-mongodb", &code, &http_err);
    if (http_err) {
        log__->Error(std::string{"http error when discover database server "} + " from " + GetReceiverConfig()->discovery_server
                     + " : " + http_err->Explain());
        return ReceiverErrorTemplates::kInternalServerError.Generate("http error when discover database server" +
                http_err->Explain());
    }

    if (code != HttpCode::OK) {
        log__->Error(std::string{"http error when discover database server "} + " from " + GetReceiverConfig()->discovery_server
                     + " : http code" + std::to_string((int)code));
        return ReceiverErrorTemplates::kInternalServerError.Generate("error when discover database server");
    }

    log__->Debug(std::string{"found database server "} + *uri);

    return nullptr;
}


Error RequestHandlerDb::ConnectToDbIfNeeded() const {
    if (!connected_to_db) {
        std::string uri;
        auto err = GetDatabaseServerUri(&uri);
        if (err) {
            return err;
        }
        err = db_client__->Connect(uri, db_name_);
        if (err) {
            return ReceiverErrorTemplates::kInternalServerError.Generate("error connecting to database " + err->Explain());
        }
        connected_to_db = true;
    }
    return nullptr;
}


}
