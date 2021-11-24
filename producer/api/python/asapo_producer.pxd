from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool
from libc.stdint cimport uint8_t
from libc.stdint cimport uint64_t

ctypedef unique_ptr[ErrorInterface] Error

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  cppclass CustomErrorData:
    pass
  cppclass ErrorInterface:
    string Explain()
  cppclass ErrorTemplateInterface:
    pass
  cdef bool operator==(Error lhs, ErrorTemplateInterface rhs)

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  ErrorTemplateInterface kTimeout "asapo::ProducerErrorTemplates::kTimeout"
  ErrorTemplateInterface kWrongInput "asapo::ProducerErrorTemplates::kWrongInput"
  ErrorTemplateInterface kLocalIOError "asapo::ProducerErrorTemplates::kLocalIOError"
  ErrorTemplateInterface kServerWarning "asapo::ProducerErrorTemplates::kServerWarning"
  ErrorTemplateInterface kRequestPoolIsFull "asapo::ProducerErrorTemplates::kRequestPoolIsFull"
  ErrorTemplateInterface kUnsupportedClient "asapo::ProducerErrorTemplates::kUnsupportedClient"

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  cppclass MessageData:
    uint8_t[] release()
    uint8_t[] get()
  cppclass StreamInfo:
    string Json()

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  cppclass RequestHandlerType:
    pass
  RequestHandlerType RequestHandlerType_Tcp "asapo::RequestHandlerType::kTcp"


cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  cppclass LogLevel:
    pass
  LogLevel LogLevel_None "asapo::LogLevel::None"
  LogLevel LogLevel_Error "asapo::LogLevel::Error"
  LogLevel LogLevel_Info "asapo::LogLevel::Info"
  LogLevel LogLevel_Debug "asapo::LogLevel::Debug"
  LogLevel LogLevel_Warning "asapo::LogLevel::Warning"

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  cppclass MetaIngestOp:
    pass
  MetaIngestOp kInsert "asapo::MetaIngestOp::kInsert"
  MetaIngestOp kReplace "asapo::MetaIngestOp::kReplace"
  MetaIngestOp kUpdate "asapo::MetaIngestOp::kUpdate"
  struct MetaIngestMode:
    MetaIngestOp op
    bool upsert

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  cppclass SourceType:
    pass
  cdef Error GetSourceTypeFromString(string types,SourceType * type)
  struct  SourceCredentials:
    string beamtime_id
    string beamline
    string data_source
    string user_token
    SourceType type

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  struct  MessageHeader:
    uint64_t message_id
    uint64_t data_size
    string file_name
    string user_metadata
    uint64_t dataset_substream
    uint64_t dataset_size
    bool auto_id


cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  struct  GenericRequestHeader:
    string Json()
  struct RequestCallbackPayload:
    GenericRequestHeader original_header
    MessageData data
    string response
  struct DeleteStreamOptions:
    bool delete_meta
    bool error_on_not_exist

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
  cppclass RequestCallback:
    pass


cdef extern from "asapo_wrappers.h" namespace "asapo":
    cppclass RequestCallbackCython:
      pass
    cppclass RequestCallbackCythonMemory:
      pass
    RequestCallback unwrap_callback(RequestCallbackCython, void*,void*)
    RequestCallback unwrap_callback_with_memory(RequestCallbackCythonMemory, void*,void*,void*)



cdef extern from "asapo/asapo_producer.h" namespace "asapo" nogil:
    cppclass Producer:
        @staticmethod
        unique_ptr[Producer] Create(string endpoint,uint8_t nthreads,RequestHandlerType type, SourceCredentials source,uint64_t timeout_ms, Error* error)
        Error SendFile(const MessageHeader& message_header, string file_to_send, uint64_t ingest_mode, string stream, RequestCallback callback)
        Error Send__(const MessageHeader& message_header, void* data, uint64_t ingest_mode, string stream, RequestCallback callback)
        void StopThreads__()
        void SetLogLevel(LogLevel level)
        uint64_t  GetRequestsQueueSize()
        uint64_t  GetRequestsQueueVolumeMb()
        void SetRequestsQueueLimits(uint64_t size, uint64_t volume)
        Error WaitRequestsFinished(uint64_t timeout_ms)
        Error SendStreamFinishedFlag(string stream, uint64_t last_id, string next_stream, RequestCallback callback)
        StreamInfo GetStreamInfo(string stream, uint64_t timeout_ms, Error* err)
        StreamInfo GetLastStream(uint64_t timeout_ms, Error* err)
        Error GetVersionInfo(string* client_info,string* server_info, bool* supported)
        Error DeleteStream(string stream, uint64_t timeout_ms, DeleteStreamOptions options)
        Error SendBeamtimeMetadata(string metadata, MetaIngestMode mode, RequestCallback callback)
        Error SendStreamMetadata(string metadata, MetaIngestMode mode, string stream, RequestCallback callback)
        string GetStreamMeta(string stream, uint64_t timeout_ms, Error* err)
        string GetBeamtimeMeta(uint64_t timeout_ms, Error* err)

cdef extern from "asapo/asapo_producer.h" namespace "asapo":
    uint64_t kDefaultIngestMode
    enum IngestModeFlags:
        kTransferData
        kTransferMetaDataOnly
        kStoreInFilesystem
        kStoreInDatabase
        kWriteRawDataToOffline

