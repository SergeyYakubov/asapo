#ifndef ASAPO_REQUEST_HANDLER_DB_WRITE_H
#define ASAPO_REQUEST_HANDLER_DB_WRITE_H

#include "request_handler.h"
#include "database/database.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerDbWrite final: public RequestHandler {
  public:
    RequestHandlerDbWrite();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<Database> db_client__;
    const AbstractLogger* log__;
    std::unique_ptr<IO> io__;
 private:
    Error ConnectToDbIfNeeded() const;
    mutable bool connected_to_db = false;
    mutable std::string db_name_;
    const std::string& GetHostName(Error* err) const;
    mutable std::string hostname_;
};

}

#endif //ASAPO_REQUEST_HANDLER_DB_WRITE_H
