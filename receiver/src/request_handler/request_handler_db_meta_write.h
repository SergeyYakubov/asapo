#ifndef ASAPO_REQUEST_HANDLER_DB_META_WRITE_H
#define ASAPO_REQUEST_HANDLER_DB_META_WRITE_H

#include "request_handler.h"
#include "asapo/database/database.h"
#include "asapo/logger/logger.h"
#include "request_handler_db.h"
#include "asapo/io/io.h"


namespace asapo {

class RequestHandlerDbMetaWrite final: public RequestHandlerDb {
  public:
    RequestHandlerDbMetaWrite(std::string collection_name);
    Error ProcessRequest(Request* request) const override;
};


}

#endif //ASAPO_REQUEST_HANDLER_DB_META_WRITE_H
