#ifndef ASAPO_REQUEST_HANDLER_DB_WRITE_H
#define ASAPO_REQUEST_HANDLER_DB_WRITE_H

#include "request_handler.h"
#include "database/database.h"
#include "logger/logger.h"

#include "io/io.h"

namespace asapo {

class RequestHandlerDbWrite final: public ReceiverRequestHandler {
  public:
    RequestHandlerDbWrite();
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<Database> db_client__;
    const AbstractLogger* log__;
  private:
    FileInfo PrepareFileInfo(const Request* request) const;
    Error InsertRecordToDb(const Request* request) const;
    Error ConnectToDbIfNeeded() const;
    mutable bool connected_to_db = false;
    mutable std::string db_name_;
};

}

#endif //ASAPO_REQUEST_HANDLER_DB_WRITE_H
