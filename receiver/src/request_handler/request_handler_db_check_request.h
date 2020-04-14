#ifndef ASAPO_REQUEST_HANDLER_DB_CHECK_REQUEST_H
#define ASAPO_REQUEST_HANDLER_DB_CHECK_REQUEST_H

#include "request_handler.h"
#include "database/database.h"
#include "request_handler_db.h"
#include "io/io.h"
#include "preprocessor/definitions.h"

namespace asapo {

class RequestHandlerDbCheckRequest FINAL : public RequestHandlerDb {
  public:
    RequestHandlerDbCheckRequest(std::string collection_name_prefix);
    Error ProcessRequest(Request* request) const override;
  private:
    Error GetRecordFromDb(const Request* request, FileInfo* record) const;
    bool SameRequestInRecord(const Request* request, const FileInfo& record) const;

};

}

#endif //ASAPO_REQUEST_HANDLER_DB_CHECK_REQUEST_H
