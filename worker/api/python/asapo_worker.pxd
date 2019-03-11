from libcpp.memory cimport unique_ptr
from libcpp.string cimport string


ctypedef unsigned char uint8_t
ctypedef unsigned long uint64_t


cdef extern from "../cpp/include/asapo_worker.h" namespace "asapo":
  cppclass Error:
    pass

cdef extern from "asapo_wrappers.h" namespace "asapo":
  cdef string GetErrorString(Error* err)

cdef extern from "../cpp/include/asapo_worker.h" namespace "asapo":
  cppclass FileData:
    pass

cdef extern from "../cpp/include/asapo_worker.h" namespace "asapo":
  cppclass FileInfo:
    pass


cdef extern from "../cpp/include/asapo_worker.h" namespace "asapo":
    cdef cppclass DataBroker:
        DataBroker() except +
        Error GetNext(FileInfo* info, FileData* data)

cdef extern from "../cpp/include/asapo_worker.h" namespace "asapo":
    cdef cppclass DataBrokerFactory:
        DataBrokerFactory() except +
        unique_ptr[DataBroker] CreateServerBroker(string server_name,string source_path,string beamtime_id,string token,Error* error)


