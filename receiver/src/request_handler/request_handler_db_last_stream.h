#ifndef ASAPO_REQUEST_HANDLER_DB_LAST_STREAM_H
#define ASAPO_REQUEST_HANDLER_DB_LAST_STREAM_H

#include "request_handler_db.h"
#include "../request.h"

namespace asapo {

class RequestHandlerDbLastStream final : public RequestHandlerDb {
  public:
    RequestHandlerDbLastStream(std::string collection_name_prefix);
    Error ProcessRequest(Request* request) const override;
};

}

#endif //ASAPO_REQUEST_HANDLER_DB_LAST_STREAM_H
