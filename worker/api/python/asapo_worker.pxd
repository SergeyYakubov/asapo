from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector


ctypedef unsigned char uint8_t
ctypedef unsigned long uint64_t


cdef extern from "asapo_worker.h" namespace "asapo":
  cppclass Error:
    pass

cdef extern from "asapo_wrappers.h" namespace "asapo":
  cdef string GetErrorString(Error* err)

cdef extern from "asapo_worker.h" namespace "asapo":
  cppclass FileData:
    unique_ptr[uint8_t[]] release()
    pass

cdef extern from "asapo_worker.h" namespace "asapo":
  cppclass FileInfo:
    string Json()
  cppclass FileInfos:
    vector[FileInfo].iterator begin()
    vector[FileInfo].iterator end()



cdef extern from "asapo_worker.h" namespace "asapo":
    cdef cppclass DataBroker:
        DataBroker() except +
        void SetTimeout(uint64_t timeout_ms)
        Error GetNext(FileInfo* info, string group_id, FileData* data)
        Error GetLast(FileInfo* info, string group_id, FileData* data)
        Error GetById(uint64_t id, FileInfo* info, string group_id, FileData* data)
        uint64_t GetNDataSets(Error* err)
        Error ResetCounter(string group_id)
        string GenerateNewGroupId(Error* err)
        string GetBeamtimeMeta(Error* err)
        FileInfos QueryImages(string query, Error* err)


cdef extern from "asapo_worker.h" namespace "asapo":
    cdef cppclass DataBrokerFactory:
        DataBrokerFactory() except +
        unique_ptr[DataBroker] CreateServerBroker(string server_name,string source_path,string beamtime_id,string token,Error* error)


