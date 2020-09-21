#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include <string>

#include "receiver_error.h"
#include "common/networking.h"
#include "io/io.h"
#include "request_handler/request_handler.h"
#include "request_handler/request_handler_file_process.h"
#include "request_handler/request_handler_db_write.h"
#include "request_handler/request_handler_authorize.h"
#include "request_handler/request_handler_db_meta_write.h"
#include "request_handler/request_handler_receive_data.h"
#include "request_handler/request_handler_receive_metadata.h"
#include "request_handler/request_handler_db_check_request.h"

#include "statistics/receiver_statistics.h"
#include "data_cache.h"

#include "preprocessor/definitions.h"
namespace asapo {

using RequestHandlerList = std::vector<const ReceiverRequestHandler*>;

enum class ResponseMessageType {
    kWarning,
    kInfo
};

class Request {
  public:
    VIRTUAL Error Handle(ReceiverStatistics*);
    ~Request() = default;
    Request() = delete;
    Request(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string origin_uri,
            DataCache* cache, const RequestHandlerDbCheckRequest* db_check_handler);
    VIRTUAL void AddHandler(const ReceiverRequestHandler*);
    VIRTUAL const RequestHandlerList& GetListHandlers() const;
    VIRTUAL uint64_t GetDataSize() const;
    VIRTUAL uint64_t GetMetaDataSize() const;
    VIRTUAL uint64_t GetDataID() const;
    VIRTUAL std::string GetFileName() const;
    VIRTUAL std::string GetSubstream() const;
    VIRTUAL void* GetData() const;
    VIRTUAL Opcode GetOpCode() const;
    VIRTUAL const char* GetMessage() const;

    const std::string& GetOriginUri() const;
    VIRTUAL const std::string& GetMetaData() const;
    VIRTUAL const std::string& GetBeamtimeId() const;
    VIRTUAL void SetBeamtimeId(std::string beamtime_id);
    VIRTUAL void SetBeamline(std::string beamline);

    VIRTUAL const std::string& GetStream() const;
    VIRTUAL void SetStream(std::string stream);
    VIRTUAL void SetMetadata(std::string metadata);

    VIRTUAL void SetOnlinePath(std::string facility);
    VIRTUAL void SetOfflinePath(std::string path);
    VIRTUAL const std::string& GetOnlinePath() const;
    VIRTUAL const std::string& GetOfflinePath() const;

    VIRTUAL const std::string& GetBeamline() const;
    VIRTUAL const CustomRequestData& GetCustomData() const;
    VIRTUAL Error PrepareDataBufferAndLockIfNeeded();
    VIRTUAL void UnlockDataBufferIfNeeded();
    VIRTUAL  SocketDescriptor GetSocket() const ;
    std::unique_ptr<IO> io__;
    DataCache* cache__ = nullptr;
    VIRTUAL uint64_t GetSlotId() const;
    VIRTUAL bool WasAlreadyProcessed() const;
    VIRTUAL void SetAlreadyProcessedFlag();
    VIRTUAL void SetResponseMessage(std::string message, ResponseMessageType type);
    VIRTUAL const ResponseMessageType GetResponseMessageType() const;
    VIRTUAL const std::string& GetResponseMessage() const;
    VIRTUAL Error CheckForDuplicates();
  private:
    const GenericRequestHeader request_header_;
    const SocketDescriptor socket_fd_;
    FileData data_buffer_;
    void* data_ptr;
    RequestHandlerList handlers_;
    std::string origin_uri_;
    std::string beamtime_id_;
    std::string stream_;
    std::string beamline_;
    std::string offline_path_;
    std::string online_path_;
    std::string metadata_;
    CacheMeta* slot_meta_ = nullptr;
    bool already_processed_ = false;
    std::string response_message_;
    ResponseMessageType response_message_type_;
    const RequestHandlerDbCheckRequest* check_duplicate_request_handler_;
};


}

#endif //ASAPO_REQUEST_H

