#ifndef ASAPO_REQUEST_HANDLER_DB_WRITE_H
#define ASAPO_REQUEST_HANDLER_DB_WRITE_H

#include "request_handler.h"
#include "database/database.h"
#include "logger/logger.h"
#include "request_handler_db.h"
#include "io/io.h"

namespace asapo {

class RequestHandlerDbWrite final: public RequestHandlerDb {
  public:
    Error ProcessRequest(Request* request) const override;
    RequestHandlerDbWrite(std::string collection_name_prefix);
  private:
    FileInfo PrepareFileInfo(const Request* request) const;
    Error InsertRecordToDb(const Request* request) const;
    Error ProcessDuplicateRecordSituation(Request* request) const;

};

}

#endif //ASAPO_REQUEST_HANDLER_DB_WRITE_H
