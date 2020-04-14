#ifndef ASAPO_REQUEST_HANDLER_DB_H
#define ASAPO_REQUEST_HANDLER_DB_H

#include "request_handler.h"
#include "database/database.h"
#include "logger/logger.h"
#include "http_client/http_client.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerDb : public ReceiverRequestHandler {
  public:
    RequestHandlerDb() = delete;
    RequestHandlerDb(std::string collection_name_prefix);
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<Database> db_client__;
    const AbstractLogger* log__;
    std::unique_ptr<HttpClient> http_client__;
  protected:
    Error ConnectToDbIfNeeded() const;
    mutable bool connected_to_db = false;
    mutable std::string db_name_;
    std::string collection_name_prefix_;
  private:
    Error GetDatabaseServerUri(std::string* uri) const ;
};

}

#endif //ASAPO_REQUEST_HANDLER_DB_H
