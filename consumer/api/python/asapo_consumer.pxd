from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool


ctypedef unsigned char uint8_t
ctypedef unsigned long uint64_t

ctypedef unique_ptr[ErrorInterface] Error

cdef extern from "asapo_consumer.h" namespace "asapo":
  cppclass CustomErrorData:
    pass
  cppclass ErrorInterface:
    string Explain()
    const CustomErrorData* GetCustomData()
  cppclass ErrorTemplateInterface:
    pass
  cdef bool operator==(Error lhs, ErrorTemplateInterface rhs)


cdef extern from "asapo_wrappers.h" namespace "asapo":
  cdef string GetErrorString(Error* err)

cdef extern from "asapo_consumer.h" namespace "asapo":
  cppclass FileData:
    unique_ptr[uint8_t[]] release()
    pass

cdef extern from "asapo_consumer.h" namespace "asapo":
  cppclass FileInfo:
    string Json()
    bool SetFromJson(string json_str)
  cppclass FileInfos:
    vector[FileInfo].iterator begin()
    vector[FileInfo].iterator end()
  struct DataSet:
    uint64_t id
    FileInfos content
  struct  SourceCredentials:
    string beamtime_id
    string stream
    string user_token

cdef extern from "asapo_consumer.h" namespace "asapo" nogil:
    cdef cppclass DataBroker:
        DataBroker() except +
        void SetTimeout(uint64_t timeout_ms)
        Error GetNext(FileInfo* info, string group_id, FileData* data)
        Error GetLast(FileInfo* info, string group_id, FileData* data)
        Error GetById(uint64_t id, FileInfo* info, string group_id, FileData* data)
        uint64_t GetCurrentSize(Error* err)
        Error SetLastReadMarker(uint64_t value, string group_id)
        Error ResetLastReadMarker(string group_id)
        string GenerateNewGroupId(Error* err)
        string GetBeamtimeMeta(Error* err)
        FileInfos QueryImages(string query, Error* err)
        DataSet GetNextDataset(string group_id, Error* err)
        DataSet GetLastDataset(string group_id, Error* err)
        DataSet GetDatasetById(uint64_t id,string group_id, Error* err)
        Error RetrieveData(FileInfo* info, FileData* data)


cdef extern from "asapo_consumer.h" namespace "asapo" nogil:
    cdef cppclass DataBrokerFactory:
        DataBrokerFactory() except +
        unique_ptr[DataBroker] CreateServerBroker(string server_name,string source_path,SourceCredentials source,Error* error)


cdef extern from "asapo_consumer.h" namespace "asapo":
  ErrorTemplateInterface kNoData "asapo::ConsumerErrorTemplates::kNoData"
  ErrorTemplateInterface kEndOfStream "asapo::ConsumerErrorTemplates::kEndOfStream"
  ErrorTemplateInterface kBrokerServersNotFound "asapo::ConsumerErrorTemplates::kBrokerServersNotFound"
  ErrorTemplateInterface kBrokerServerError "asapo::ConsumerErrorTemplates::kBrokerServerError"
  ErrorTemplateInterface kIOError "asapo::ConsumerErrorTemplates::kIOError"
  ErrorTemplateInterface kWrongInput "asapo::ConsumerErrorTemplates::kWrongInput"
  cdef cppclass ConsumerErrorData:
    uint64_t id
    uint64_t id_max

