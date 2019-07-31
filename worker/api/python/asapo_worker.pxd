from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool


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
    bool SetFromJson(string json_str)
  cppclass FileInfos:
    vector[FileInfo].iterator begin()
    vector[FileInfo].iterator end()
  struct DataSet:
    uint64_t id
    FileInfos content


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
        DataSet GetNextDataset(string group_id, Error* err)
        DataSet GetLastDataset(string group_id, Error* err)
        DataSet GetDatasetById(uint64_t id,string group_id, Error* err)
        Error RetrieveData(FileInfo* info, FileData* data)


cdef extern from "asapo_worker.h" namespace "asapo":
    cdef cppclass DataBrokerFactory:
        DataBrokerFactory() except +
        unique_ptr[DataBroker] CreateServerBroker(string server_name,string source_path,string beamtime_id,string token,Error* error)

