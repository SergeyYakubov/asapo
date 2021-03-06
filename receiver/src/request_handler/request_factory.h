#ifndef ASAPO_REQUEST_FACTORY_H
#define ASAPO_REQUEST_FACTORY_H

#include "../request.h"
#include "./file_processors/write_file_processor.h"
#include "./file_processors/receive_file_processor.h"
#include "request_handler_db_stream_info.h"
#include "request_handler_db_last_stream.h"
#include "request_handler_db_delete_stream.h"
#include "request_handler_db_get_meta.h"

#include "request_handler_file_process.h"
#include "request_handler_db_write.h"
#include "request_handler_initial_authorization.h"
#include "request_handler_secondary_authorization.h"
#include "request_handler_db_meta_write.h"
#include "request_handler_receive_data.h"
#include "request_handler_receive_metadata.h"
#include "request_handler_db_check_request.h"
#include "structs.h"

namespace asapo {

class RequestFactory {
  public:
    explicit RequestFactory (SharedCache cache);
    virtual std::unique_ptr<Request> GenerateRequest(const GenericRequestHeader& request_header,
                                                     SocketDescriptor socket_fd, std::string origin_uri, Error* err) const noexcept;
    virtual ~RequestFactory() = default;
  private:
    Error AddHandlersToRequest(std::unique_ptr<Request>& request,  const GenericRequestHeader& request_header) const;
    Error AddReceiveWriteHandlers(std::unique_ptr<Request>& request, const GenericRequestHeader& request_header) const;
    WriteFileProcessor write_file_processor_;
    ReceiveFileProcessor receive_file_processor_;
    RequestHandlerFileProcess request_handler_filewrite_{&write_file_processor_};
    RequestHandlerFileProcess request_handler_filereceive_{&receive_file_processor_};
    RequestHandlerReceiveData request_handler_receivedata_;
    RequestHandlerReceiveMetaData request_handler_receive_metadata_;
    RequestHandlerDbWrite request_handler_dbwrite_{kDBDataCollectionNamePrefix};
    RequestHandlerDbStreamInfo request_handler_db_stream_info_{kDBDataCollectionNamePrefix};
    RequestHandlerDbDeleteStream request_handler_delete_stream_{kDBDataCollectionNamePrefix};
    RequestHandlerDbLastStream request_handler_db_last_stream_{kDBDataCollectionNamePrefix};
    RequestHandlerDbMetaWrite request_handler_db_meta_write_{kDBMetaCollectionName};
    RequestHandlerDbGetMeta request_handler_db_get_meta_{kDBMetaCollectionName};
    RequestHandlerInitialAuthorization request_handler_initial_authorize_{&shared_auth_cache_};
    RequestHandlerSecondaryAuthorization request_handler_secondary_authorize_{&shared_auth_cache_};
    RequestHandlerDbCheckRequest request_handler_db_check_{kDBDataCollectionNamePrefix};
    SharedCache cache_;
    AuthorizationData shared_auth_cache_;
    bool ReceiveDirectToFile(const GenericRequestHeader& request_header) const;
    Error AddReceiveDirectToFileHandler(std::unique_ptr<Request>& request,
                                        const GenericRequestHeader& request_header) const;
    void AddReceiveViaBufferHandlers(std::unique_ptr<Request>& request,
                                     const GenericRequestHeader& request_header) const;
};

}

#endif //ASAPO_REQUEST_FACTORY_H
