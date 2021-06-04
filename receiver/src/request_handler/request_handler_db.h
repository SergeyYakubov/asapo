#ifndef ASAPO_REQUEST_HANDLER_DB_H
#define ASAPO_REQUEST_HANDLER_DB_H

#include "request_handler.h"
#include "asapo/database/database.h"
#include "asapo/logger/logger.h"
#include "asapo/http_client/http_client.h"

#include "asapo/io/io.h"

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
    Error DBErrorToReceiverError(const Error& err) const;
    mutable bool connected_to_db = false;
    mutable std::string db_name_;
    std::string collection_name_prefix_;
  private:
    Error GetDatabaseServerUri(std::string* uri) const ;
};

}

#endif //ASAPO_REQUEST_HANDLER_DB_H
