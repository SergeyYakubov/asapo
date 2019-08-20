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
  cdef string GetErrorString(Error* err)


cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass FileData:
    unique_ptr[uint8_t[]] release()


cdef extern from "asapo_producer.h" namespace "asapo":
  cppclass RequestHandlerType:
    pass
  cdef RequestHandlerType RequestHandlerType_Tcp "asapo::RequestHandlerType::kTcp"



cdef extern from "asapo_producer.h" namespace "asapo":
  struct  SourceCredentials:
    string beamtime_id
    string stream
    string user_token

cdef extern from "asapo_producer.h" namespace "asapo":
    cdef cppclass Producer:
        @staticmethod
        unique_ptr[Producer] Create(string endpoint,uint8_t nthreads,RequestHandlerType type, SourceCredentials source,Error* error)
