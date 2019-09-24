from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

ctypedef unsigned char uint8_t
ctypedef unsigned long uint64_t

ctypedef unique_ptr[ErrorInterface] Error

cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass CustomErrorData:
    pass
  cppclass ErrorInterface:
    string Explain()
  cppclass ErrorTemplateInterface:
    pass
  cdef bool operator==(Error lhs, ErrorTemplateInterface rhs)

cdef extern from "asapo_producer.h" namespace "asapo":
  ErrorTemplateInterface kFileTooLarge "asapo::ProducerErrorTemplates::kFileTooLarge"
  ErrorTemplateInterface kFileNameTooLong "asapo::ProducerErrorTemplates::kFileNameTooLong"
  ErrorTemplateInterface kEmptyFileName "asapo::ProducerErrorTemplates::kEmptyFileName"
  ErrorTemplateInterface kNoData "asapo::ProducerErrorTemplates::kNoData"
  ErrorTemplateInterface kZeroDataSize "asapo::ProducerErrorTemplates::kZeroDataSize"
  ErrorTemplateInterface kBeamtimeIdTooLong "asapo::ProducerErrorTemplates::kBeamtimeIdTooLong"
  ErrorTemplateInterface kBeamtimeAlreadySet "asapo::ProducerErrorTemplates::kBeamtimeAlreadySet"
  ErrorTemplateInterface kFileIdAlreadyInUse "asapo::ProducerErrorTemplates::kFileIdAlreadyInUse"
  ErrorTemplateInterface kErrorInMetadata "asapo::ProducerErrorTemplates::kErrorInMetadata"
  ErrorTemplateInterface kErrorSubsetSize "asapo::ProducerErrorTemplates::kErrorSubsetSize"
  ErrorTemplateInterface kAuthorizationFailed "asapo::ProducerErrorTemplates::kAuthorizationFailed"
  ErrorTemplateInterface kInternalServerError "asapo::ProducerErrorTemplates::kInternalServerError"
  ErrorTemplateInterface kCannotSendDataToReceivers "asapo::ProducerErrorTemplates::kCannotSendDataToReceivers"
  ErrorTemplateInterface kRequestPoolIsFull "asapo::ProducerErrorTemplates::kRequestPoolIsFull"
  ErrorTemplateInterface kWrongIngestMode "asapo::ProducerErrorTemplates::kWrongIngestMode"

cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass FileData:
    unique_ptr[uint8_t[]] release()


cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass RequestHandlerType:
    pass
  RequestHandlerType RequestHandlerType_Tcp "asapo::RequestHandlerType::kTcp"


cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass LogLevel:
    pass
  LogLevel LogLevel_None "asapo::LogLevel::None"
  LogLevel LogLevel_Error "asapo::LogLevel::Error"
  LogLevel LogLevel_Info "asapo::LogLevel::Info"
  LogLevel LogLevel_Debug "asapo::LogLevel::Debug"
  LogLevel LogLevel_Warning "asapo::LogLevel::Warning"


cdef extern from "asapo_producer.h" namespace "asapo":
  struct  SourceCredentials:
    string beamtime_id
    string stream
    string user_token

cdef extern from "asapo_producer.h" namespace "asapo":
  struct  EventHeader:
    uint64_t file_id
    uint64_t file_size
    string file_name
    string user_metadata
    uint64_t subset_id
    uint64_t subset_size

cdef extern from "asapo_producer.h" namespace "asapo":
  struct  EventHeader:
    uint64_t file_id
    uint64_t file_size
    string file_name
    string user_metadata
    uint64_t subset_id
    uint64_t subset_size

cdef extern from "asapo_producer.h" namespace "asapo":
  struct  GenericRequestHeader:
    string Json()

cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass RequestCallback:
    pass


cdef extern from "asapo_wrappers.h" namespace "asapo":
    cppclass RequestCallbackCython:
      pass
    cppclass RequestCallbackCythonMemory:
      pass
    RequestCallback unwrap_callback(RequestCallbackCython, void*,void*)
    RequestCallback unwrap_callback_with_memory(RequestCallbackCythonMemory, void*,void*,void*)


cdef extern from "asapo_producer.h" namespace "asapo":
    cppclass Producer:
        @staticmethod
        unique_ptr[Producer] Create(string endpoint,uint8_t nthreads,RequestHandlerType type, SourceCredentials source,Error* error)
        Error SendFile(const EventHeader& event_header, string full_path, uint64_t ingest_mode,RequestCallback callback)
        Error SendData_(const EventHeader& event_header, void* data, uint64_t ingest_mode,RequestCallback callback)
        void SetLogLevel(LogLevel level)

cdef extern from "asapo_producer.h" namespace "asapo":
    uint64_t kDefaultIngestMode
    enum IngestModeFlags:
        kTransferData
        kTransferMetaDataOnly
        kStoreInFilesystem
