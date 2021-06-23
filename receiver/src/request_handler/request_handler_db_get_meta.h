#include "request_handler_db.h"
#include "../request.h"

#ifndef ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_DB_GET_META_H_
#define ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_DB_GET_META_H_

namespace asapo {

class RequestHandlerDbGetMeta final: public RequestHandlerDb {
  public:
    RequestHandlerDbGetMeta(std::string collection_name_prefix);
    Error ProcessRequest(Request* request) const override;
};

}


#endif //ASAPO_RECEIVER_SRC_REQUEST_HANDLER_REQUEST_HANDLER_DB_GET_META_H_
