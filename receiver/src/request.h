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
#include "request_handler_db_meta_write.h"


#include "receiver_statistics.h"
#include "data_cache.h"

#include "preprocessor/definitions.h"
namespace asapo {

using RequestHandlerList = std::vector<const ReceiverRequestHandler*>;

class Request {
  public:
    VIRTUAL Error Handle(ReceiverStatistics*);
    ~Request() = default;
    Request(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string origin_uri,
            DataCache* cache);
    VIRTUAL void AddHandler(const ReceiverRequestHandler*);
    VIRTUAL const RequestHandlerList& GetListHandlers() const;
    VIRTUAL uint64_t GetDataSize() const;
    VIRTUAL uint64_t GetDataID() const;
    VIRTUAL std::string GetFileName() const;
    VIRTUAL void* GetData() const;
    VIRTUAL Opcode GetOpCode() const;
    VIRTUAL const char* GetMessage() const;

    const std::string& GetOriginUri() const;
    VIRTUAL const std::string& GetMetaData() const;
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
    Error ReceiveMetaData();
    Error ReceiveRequestContent(ReceiverStatistics* statistics);

    const GenericRequestHeader request_header_;
    const SocketDescriptor socket_fd_;
    FileData data_buffer_;
    void* data_ptr;
    RequestHandlerList handlers_;
    std::string origin_uri_;
    std::string beamtime_id_;
    std::string beamline_;
    std::string metadata_;
    CacheMeta* slot_meta_ = nullptr;
};


}

#endif //ASAPO_REQUEST_H

