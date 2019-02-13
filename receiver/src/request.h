#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include <string>

#include "receiver_error.h"
#include "common/networking.h"
#include "io/io.h"
#include "request_handler.h"
#include "request_handler_file_write.h"
#include "request_handler_db_write.h"
#include "request_handler_authorize.h"

#include "statistics.h"
#include "data_cache.h"

#include "preprocessor/definitions.h"
namespace asapo {

using RequestHandlerList = std::vector<const RequestHandler*>;

class Request {
  public:
    VIRTUAL Error Handle(Statistics*);
    ~Request() = default;
    Request(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string origin_uri,
            DataCache* cache);
    VIRTUAL void AddHandler(const RequestHandler*);
    VIRTUAL const RequestHandlerList& GetListHandlers() const;
    VIRTUAL uint64_t GetDataSize() const;
    VIRTUAL uint64_t GetDataID() const;
    VIRTUAL std::string GetFileName() const;
    VIRTUAL void* GetData() const;
    VIRTUAL Opcode GetOpCode() const;
    VIRTUAL const char* GetMessage() const;

    const std::string& GetOriginUri() const;
    VIRTUAL const std::string& GetBeamtimeId() const;
    VIRTUAL void SetBeamtimeId(std::string beamtime_id);
    VIRTUAL void SetBeamline(std::string beamline);
    VIRTUAL const std::string& GetBeamline() const;
    std::unique_ptr<IO> io__;
    DataCache* cache__ = nullptr;
    VIRTUAL uint64_t GetSlotId() const;
  private:
    Error PrepareDataBuffer();
    Error ReceiveData();
    const GenericRequestHeader request_header_;
    const SocketDescriptor socket_fd_;
    FileData data_buffer_;
    void* data_ptr;
    RequestHandlerList handlers_;
    std::string origin_uri_;
    std::string beamtime_id_;
    std::string beamline_;
    uint64_t slot_id_{0};
};

class RequestFactory {
  public:
    explicit RequestFactory (SharedCache cache);
    virtual std::unique_ptr<Request> GenerateRequest(const GenericRequestHeader& request_header,
                                                     SocketDescriptor socket_fd, std::string origin_uri, Error* err) const noexcept;
  private:
    RequestHandlerFileWrite request_handler_filewrite_;
    RequestHandlerDbWrite request_handler_dbwrite_;
    RequestHandlerAuthorize request_handler_authorize_;
    SharedCache cache_;
};

}

#endif //ASAPO_REQUEST_H

