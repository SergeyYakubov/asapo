#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include <string>

#include "receiver_error.h"
#include "asapo/common/networking.h"
#include "asapo/io/io.h"
#include "request_handler/request_handler.h"

#include "statistics/receiver_statistics.h"
#include "data_cache.h"

#include "asapo/preprocessor/definitions.h"
#include "statistics/instanced_statistics_provider.h"
#include "asapo/logger/logger.h"

namespace asapo {

using RequestHandlerList = std::vector<const ReceiverRequestHandler*>;

enum class ResponseMessageType {
    kWarning,
    kInfo
};

class RequestHandlerDbCheckRequest;

class Request {
  public:
    ASAPO_VIRTUAL Error Handle();
    ASAPO_VIRTUAL ~Request() = default;
    Request() = delete;
    Request(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string origin_uri,
            DataCache* cache, const RequestHandlerDbCheckRequest* db_check_handler,
            RequestStatisticsPtr  statistics);
    ASAPO_VIRTUAL void AddHandler(const ReceiverRequestHandler*);
    ASAPO_VIRTUAL const RequestHandlerList& GetListHandlers() const;
    ASAPO_VIRTUAL uint64_t GetDataSize() const;
    ASAPO_VIRTUAL uint64_t GetMetaDataSize() const;
    ASAPO_VIRTUAL uint64_t GetDataID() const;
    ASAPO_VIRTUAL std::string GetFileName() const;
    ASAPO_VIRTUAL std::string GetStream() const;
    ASAPO_VIRTUAL std::string GetApiVersion() const;
    ASAPO_VIRTUAL void* GetData() const;
    ASAPO_VIRTUAL Opcode GetOpCode() const;
    ASAPO_VIRTUAL const char* GetMessage() const;

    ASAPO_VIRTUAL const std::string& GetProducerInstanceId() const;
    ASAPO_VIRTUAL void SetProducerInstanceId(std::string producer_instance_id);
    ASAPO_VIRTUAL const std::string& GetPipelineStepId() const;
    ASAPO_VIRTUAL void SetPipelineStepId(std::string pipeline_step_id);
    ASAPO_VIRTUAL const std::string& GetOriginUri() const;
    ASAPO_VIRTUAL const std::string& GetOriginHost() const;
    ASAPO_VIRTUAL const std::string& GetMetaData() const;
    ASAPO_VIRTUAL const std::string& GetBeamtimeId() const;
    ASAPO_VIRTUAL void SetBeamtimeId(std::string beamtime_id);
    ASAPO_VIRTUAL void SetBeamline(std::string beamline);

    ASAPO_VIRTUAL void SetSourceType(SourceType);
    ASAPO_VIRTUAL SourceType GetSourceType() const;

    ASAPO_VIRTUAL const std::string& GetDataSource() const;
    ASAPO_VIRTUAL void SetDataSource(std::string data_source);
    ASAPO_VIRTUAL void SetMetadata(std::string metadata);

    ASAPO_VIRTUAL void SetOnlinePath(std::string facility);
    ASAPO_VIRTUAL void SetOfflinePath(std::string path);
    ASAPO_VIRTUAL const std::string& GetOnlinePath() const;
    ASAPO_VIRTUAL const std::string& GetOfflinePath() const;

    ASAPO_VIRTUAL const std::string& GetBeamline() const;
    ASAPO_VIRTUAL const CustomRequestData& GetCustomData() const;
    ASAPO_VIRTUAL Error PrepareDataBufferAndLockIfNeeded();
    ASAPO_VIRTUAL void UnlockDataBufferIfNeeded();
    ASAPO_VIRTUAL  SocketDescriptor GetSocket() const ;
    std::unique_ptr<IO> io__;
    DataCache* cache__ = nullptr;
    ASAPO_VIRTUAL uint64_t GetSlotId() const;
    ASAPO_VIRTUAL bool WasAlreadyProcessed() const;
    ASAPO_VIRTUAL void SetAlreadyProcessedFlag();
    ASAPO_VIRTUAL void SetResponseMessage(std::string message, ResponseMessageType type);
    ASAPO_VIRTUAL ResponseMessageType GetResponseMessageType() const;
    ASAPO_VIRTUAL const std::string& GetResponseMessage() const;
    ASAPO_VIRTUAL Error CheckForDuplicates();
    ASAPO_VIRTUAL RequestStatistics* GetStatistics();
    const AbstractLogger* log__;
 private:
    Error PrepareDataBufferFromMemory();
    Error PrepareDataBufferFromCache();
  private:
    RequestStatisticsPtr statistics_;
    const GenericRequestHeader request_header_;
    const SocketDescriptor socket_fd_;
    MessageData data_buffer_;
    void* data_ptr;
    RequestHandlerList handlers_;
    std::string producer_instance_id_;
    std::string pipeline_step_id_;
    std::string origin_uri_;
    std::string origin_host_;
    std::string beamtime_id_;
    std::string data_source_;
    std::string beamline_;
    std::string offline_path_;
    std::string online_path_;
    std::string metadata_;
    CacheMeta* slot_meta_ = nullptr;
    bool already_processed_ = false;
    std::string response_message_;
    ResponseMessageType response_message_type_;
    const RequestHandlerDbCheckRequest* check_duplicate_request_handler_;
    SourceType source_type_ = SourceType::kProcessed;
};

}

#endif //ASAPO_REQUEST_H

