from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool
from libc.stdint cimport uint8_t
from libc.stdint cimport uint64_t


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
  cppclass IdList:
    vector[uint64_t].iterator begin()
    vector[uint64_t].iterator end()
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
        Error GetNext(FileInfo* info, string group_id,string substream, FileData* data)
        Error GetLast(FileInfo* info, string group_id,string substream, FileData* data)
        Error GetById(uint64_t id, FileInfo* info, string group_id,string substream, FileData* data)
        uint64_t GetCurrentSize(string substream, Error* err)
        Error SetLastReadMarker(uint64_t value, string group_id,string substream)
        Error ResetLastReadMarker(string group_id,string substream)
        Error Acknowledge(string group_id, uint64_t id, string substream)
        uint64_t GetLastAcknowledgedTulpeId(string group_id, string substream, Error* error)
        IdList GetUnacknowledgedTupleIds(string group_id, string substream, uint64_t from_id, uint64_t to_id, Error* error)
        string GenerateNewGroupId(Error* err)
        string GetBeamtimeMeta(Error* err)
        FileInfos QueryImages(string query,string substream, Error* err)
        DataSet GetNextDataset(string group_id,string substream, Error* err)
        DataSet GetLastDataset(string group_id,string substream, Error* err)
        DataSet GetDatasetById(uint64_t id,string group_id,string substream, Error* err)
        Error RetrieveData(FileInfo* info, FileData* data)
        vector[string] GetSubstreamList(Error* err)
        void SetResendNacs(bool resend, uint64_t resend_after, uint64_t resend_attempts)


cdef extern from "asapo_consumer.h" namespace "asapo" nogil:
    cdef cppclass DataBrokerFactory:
        DataBrokerFactory() except +
        unique_ptr[DataBroker] CreateServerBroker(string server_name,string source_path,bool has_filesystem,SourceCredentials source,Error* error)


cdef extern from "asapo_consumer.h" namespace "asapo":
  ErrorTemplateInterface kNoData "asapo::ConsumerErrorTemplates::kNoData"
  ErrorTemplateInterface kEndOfStream "asapo::ConsumerErrorTemplates::kEndOfStream"
  ErrorTemplateInterface kStreamFinished "asapo::ConsumerErrorTemplates::kStreamFinished"
  ErrorTemplateInterface kUnavailableService "asapo::ConsumerErrorTemplates::kUnavailableService"
  ErrorTemplateInterface kInterruptedTransaction "asapo::ConsumerErrorTemplates::kInterruptedTransaction"
  ErrorTemplateInterface kLocalIOError "asapo::ConsumerErrorTemplates::kLocalIOError"
  ErrorTemplateInterface kWrongInput "asapo::ConsumerErrorTemplates::kWrongInput"
  cdef cppclass ConsumerErrorData:
    uint64_t id
    uint64_t id_max
    string next_substream

