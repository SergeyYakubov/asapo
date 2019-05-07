#ifndef ASAPO_REQUEST_HANDLER_DB_META_WRITE_H
#define ASAPO_REQUEST_HANDLER_DB_META_WRITE_H

#include "request_handler.h"
#include "database/database.h"
#include "logger/logger.h"
#include "request_handler_db.h"
#include "io/io.h"


namespace asapo {

class RequestHandlerDbMetaWrite final: public RequestHandlerDb {
  public:
    RequestHandlerDbMetaWrite(std::string collection_name);
    Error ProcessRequest(Request* request) const override;
};


}

#endif //ASAPO_REQUEST_HANDLER_DB_META_WRITE_H
