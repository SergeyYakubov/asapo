from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

ctypedef unsigned char uint8_t
ctypedef unsigned long uint64_t

cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass Error:
    pass


cdef extern from "asapo_wrappers.h" namespace "asapo":
    string GetErrorString(Error* err)


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
    RequestCallback unwrap_callback(RequestCallbackCython, void*,void*)


cdef extern from "asapo_producer.h" namespace "asapo":
    cppclass Producer:
        @staticmethod
        unique_ptr[Producer] Create(string endpoint,uint8_t nthreads,RequestHandlerType type, SourceCredentials source,Error* error)
        Error SendFile(const EventHeader& event_header, string full_path, uint64_t injest_mode,RequestCallback callback)
        void SetLogLevel(LogLevel level)

cdef extern from "asapo_producer.h" namespace "asapo":
    uint64_t kDefaultIngestMode
    enum IngestModeFlags:
        kTransferData
        kTransferMetaDataOnly
        kStoreInFilesystem

