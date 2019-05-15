#ifndef ASAPO_REQUEST_HANDLER_DB_H
#define ASAPO_REQUEST_HANDLER_DB_H

#include "request_handler.h"
#include "database/database.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerDb: public ReceiverRequestHandler {
  public:
    RequestHandlerDb() = delete;
    RequestHandlerDb(std::string collection_name);
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<Database> db_client__;
    const AbstractLogger* log__;
  protected:
    Error ConnectToDbIfNeeded() const;
    mutable bool connected_to_db = false;
    mutable std::string db_name_;
    std::string collection_name_;
};

}


#endif //ASAPO_REQUEST_HANDLER_DB_H
