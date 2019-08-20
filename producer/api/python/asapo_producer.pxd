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
    pass

cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass RequestCallback:
    pass


cdef extern from "asapo_wrappers.h" namespace "asapo":
    cdef cppclass function_wrapper:
        ctypedef void (*cy_callback) (void*, GenericRequestHeader, Error)
        @staticmethod
        RequestCallback make_std_function(cy_callback, void*)


cdef extern from "asapo_producer.h" namespace "asapo":
    cppclass Producer:
        @staticmethod
        unique_ptr[Producer] Create(string endpoint,uint8_t nthreads,RequestHandlerType type, SourceCredentials source,Error* error)
        Error SendFile(const EventHeader& event_header, string full_path, uint64_t injest_mode,RequestCallback callback)

