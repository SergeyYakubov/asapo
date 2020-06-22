#ifndef ASAPO_REQUEST_HANDLER_DB_STREAM_INFO_H
#define ASAPO_REQUEST_HANDLER_DB_STREAM_INFO_H

#include "request_handler_db.h"
#include "../request.h"

namespace asapo {

class RequestHandlerDbStreamInfo final: public RequestHandlerDb {
 public:
  RequestHandlerDbStreamInfo(std::string collection_name_prefix);
  Error ProcessRequest(Request* request) const override;
};

}

#endif //ASAPO_REQUEST_HANDLER_DB_STREAM_INFO_H
