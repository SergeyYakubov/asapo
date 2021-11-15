#include "request_handler_db.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"
#include "asapo/database/db_error.h"

namespace asapo {

Error RequestHandlerDb::ProcessRequest(Request* request) const {
    if (db_name_.empty()) {
        db_name_ = request->GetBeamtimeId();
        auto data_source = request->GetDataSource();
        db_name_ += "_" + data_source;
    }

    return ConnectToDbIfNeeded();
}

RequestHandlerDb::RequestHandlerDb(std::string collection_name_prefix) : log__{GetDefaultReceiverLogger()},
    http_client__{DefaultHttpClient()},
    collection_name_prefix_{
    std::move(collection_name_prefix)} {
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
        log__->Error(LogMessageWithFields("http error while discovering database server: " + http_err->Explain()).
                     Append("origin", GetReceiverConfig()->discovery_server));
        auto err = ReceiverErrorTemplates::kInternalServerError.Generate("http error while discovering database server",
                std::move(http_err));
        err->AddDetails("discoveryEndpoint", GetReceiverConfig()->discovery_server);
        return err;
    }

    if (code != HttpCode::OK) {
        auto err =  ReceiverErrorTemplates::kInternalServerError.Generate("error when discover database server");
        err->AddDetails("discoveryEndpoint", GetReceiverConfig()->discovery_server)->AddDetails("errorCode",
                                                                                                std::to_string((int) code));
        return err;
    }

    log__->Debug(LogMessageWithFields("discovered database").Append("server",*uri));
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
            return DBErrorToReceiverError(std::move(err));
        }
        connected_to_db = true;
    }
    return nullptr;
}

Error RequestHandlerDb::DBErrorToReceiverError(Error err) const {
    if (err == nullptr) {
        return nullptr;
    }
    Error return_err;
    if (err == DBErrorTemplates::kWrongInput || err == DBErrorTemplates::kNoRecord
            || err == DBErrorTemplates::kJsonParseError) {
        return_err = ReceiverErrorTemplates::kBadRequest.Generate("error from database");
    } else {
        return_err = ReceiverErrorTemplates::kInternalServerError.Generate("error from database");
    }
    return_err->SetCause(std::move(err));
    return return_err;
}

}
