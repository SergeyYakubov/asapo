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
    Error AddHandlersToRequest(std::unique_ptr<Request>& request,  const GenericRequestHeader& request_header) const;
    Error AddReceiveWriteHandlers(std::unique_ptr<Request>& request, const GenericRequestHeader& request_header) const;
    RequestHandlerFileWrite request_handler_filewrite_;
    RequestHandlerReceiveData request_handler_receivedata_;
    RequestHandlerReceiveMetaData request_handler_receive_metadata_;
    RequestHandlerDbWrite request_handler_dbwrite_{kDBDataCollectionNamePrefix};
    RequestHandlerDbMetaWrite request_handler_db_meta_write_{kDBMetaCollectionName};
    RequestHandlerAuthorize request_handler_authorize_;
    RequestHandlerFileReceive request_handler_filereceive_;
    SharedCache cache_;
    bool ReceiveDirectToFile(const GenericRequestHeader& request_header) const;
    Error AddReceiveDirectToFileHandler(std::unique_ptr<Request>& request,
                                        const GenericRequestHeader& request_header) const;
    void AddReceiveViaBufferHandlers(std::unique_ptr<Request>& request,
                                     const GenericRequestHeader& request_header) const;
};

}

#endif //ASAPO_REQUEST_FACTORY_H
