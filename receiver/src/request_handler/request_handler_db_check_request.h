#ifndef ASAPO_REQUEST_HANDLER_DB_CHECK_REQUEST_H
#define ASAPO_REQUEST_HANDLER_DB_CHECK_REQUEST_H

#include "request_handler.h"
#include "asapo/database/database.h"
#include "request_handler_db.h"
#include "asapo/io/io.h"
#include "asapo/preprocessor/definitions.h"

namespace asapo {

class RequestHandlerDbCheckRequest FINAL : public RequestHandlerDb {
  public:
    RequestHandlerDbCheckRequest(std::string collection_name_prefix);
    Error ProcessRequest(Request* request) const override;
  private:
    Error GetRecordFromDb(const Request* request, MessageMeta* record) const;
    bool SameRequestInRecord(const Request* request, const MessageMeta& record) const;

};

}

#endif //ASAPO_REQUEST_HANDLER_DB_CHECK_REQUEST_H
