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
    uint8_t[] release()
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
    uint64_t expected_size
    FileInfos content
  struct  SourceCredentials:
    string beamtime_id
    string stream
    string user_token
  cppclass StreamInfo:
    string Json(bool add_last_id)
    bool SetFromJson(string json_str, bool read_last_id)

cdef extern from "asapo_consumer.h" namespace "asapo":
  cppclass NetworkConnectionType:
    pass
  NetworkConnectionType NetworkConnectionType_kUndefined "asapo::NetworkConnectionType::kUndefined"
  NetworkConnectionType NetworkConnectionType_kAsapoTcp "asapo::NetworkConnectionType::kAsapoTcp"
  NetworkConnectionType NetworkConnectionType_kFabric "asapo::NetworkConnectionType::kFabric"

cdef extern from "asapo_consumer.h" namespace "asapo" nogil:
    cdef cppclass DataBroker:
        DataBroker() except +
        void SetTimeout(uint64_t timeout_ms)
        void ForceNoRdma()
        NetworkConnectionType CurrentConnectionType()
        Error GetNext(FileInfo* info, string group_id,string substream, FileData* data)
        Error GetLast(FileInfo* info, string substream, FileData* data)
        Error GetById(uint64_t id, FileInfo* info, string substream, FileData* data)
        uint64_t GetCurrentSize(string substream, Error* err)
        Error SetLastReadMarker(uint64_t value, string group_id, string substream)
        Error ResetLastReadMarker(string group_id, string substream)
        Error Acknowledge(string group_id, uint64_t id, string substream)
        Error NegativeAcknowledge(string group_id, uint64_t id, uint64_t delay_sec, string substream)
        uint64_t GetLastAcknowledgedTulpeId(string group_id, string substream, Error* error)
        IdList GetUnacknowledgedTupleIds(string group_id, string substream, uint64_t from_id, uint64_t to_id, Error* error)
        string GenerateNewGroupId(Error* err)
        string GetBeamtimeMeta(Error* err)
        FileInfos QueryImages(string query, string substream, Error* err)
        DataSet GetNextDataset(string group_id, string substream, uint64_t min_size, Error* err)
        DataSet GetLastDataset(string substream, uint64_t min_size, Error* err)
        DataSet GetDatasetById(uint64_t id, string substream, uint64_t min_size, Error* err)
        Error RetrieveData(FileInfo* info, FileData* data)
        vector[StreamInfo] GetSubstreamList(string from_substream, Error* err)
        void SetResendNacs(bool resend, uint64_t delay_sec, uint64_t resend_attempts)


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
  ErrorTemplateInterface kPartialData "asapo::ConsumerErrorTemplates::kPartialData"

  cdef cppclass ConsumerErrorData:
    uint64_t id
    uint64_t id_max
    string next_substream
