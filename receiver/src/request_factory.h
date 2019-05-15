#ifndef ASAPO_REQUEST_FACTORY_H
#define ASAPO_REQUEST_FACTORY_H

#include "request.h"

namespace asapo {

class RequestFactory {
  public:
    explicit RequestFactory (SharedCache cache);
    virtual std::unique_ptr<Request> GenerateRequest(const GenericRequestHeader& request_header,
                                                     SocketDescriptor socket_fd, std::string origin_uri, Error* err) const noexcept;
  private:
    RequestHandlerFileWrite request_handler_filewrite_;
    RequestHandlerDbWrite request_handler_dbwrite_{kDBDataCollectionName};
    RequestHandlerDbMetaWrite request_handler_db_meta_write_{kDBMetaCollectionName};
    RequestHandlerAuthorize request_handler_authorize_;
    SharedCache cache_;
};

}

#endif //ASAPO_REQUEST_FACTORY_H
