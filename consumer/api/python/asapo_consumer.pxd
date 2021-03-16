from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool
from libc.stdint cimport uint8_t
from libc.stdint cimport uint64_t


ctypedef unique_ptr[ErrorInterface] Error

cdef extern from "asapo/asapo_consumer.h" namespace "asapo":
  cppclass CustomErrorData:
    pass
  cppclass ErrorInterface:
    string Explain()
    CustomErrorData* GetCustomData()
  cppclass ErrorTemplateInterface:
    pass
  cdef bool operator==(Error lhs, ErrorTemplateInterface rhs)


cdef extern from "asapo_wrappers.h" namespace "asapo":
  cdef string GetErrorString(Error* err)

cdef extern from "asapo/asapo_consumer.h" namespace "asapo":
  cppclass MessageData:
    uint8_t[] release()
    pass

cdef extern from "asapo/asapo_consumer.h" namespace "asapo":
  cppclass MessageMeta:
    string Json()
    bool SetFromJson(string json_str)
  cppclass IdList:
    vector[uint64_t].iterator begin()
    vector[uint64_t].iterator end()
  cppclass MessageMetas:
    vector[MessageMeta].iterator begin()
    vector[MessageMeta].iterator end()
  struct DataSet:
    uint64_t id
    uint64_t expected_size
    MessageMetas content
  struct  SourceCredentials:
    string beamtime_id
    string data_source
    string user_token
  cppclass StreamInfo:
    string Json()

cdef extern from "asapo/asapo_consumer.h" namespace "asapo":
  cppclass NetworkConnectionType:
    pass
  NetworkConnectionType NetworkConnectionType_kUndefined "asapo::NetworkConnectionType::kUndefined"
  NetworkConnectionType NetworkConnectionType_kAsapoTcp "asapo::NetworkConnectionType::kAsapoTcp"
  NetworkConnectionType NetworkConnectionType_kFabric "asapo::NetworkConnectionType::kFabric"
  cppclass StreamFilter:
    pass
  StreamFilter StreamFilter_kAllStreams "asapo::StreamFilter::kAllStreams"
  StreamFilter StreamFilter_kFinishedStreams "asapo::StreamFilter::kFinishedStreams"
  StreamFilter StreamFilter_kUnfinishedStreams "asapo::StreamFilter::kUnfinishedStreams"

cdef extern from "asapo/asapo_consumer.h" namespace "asapo" nogil:
    cdef cppclass Consumer:
        Consumer() except +
        void SetTimeout(uint64_t timeout_ms)
        void ForceNoRdma()
        NetworkConnectionType CurrentConnectionType()
        Error GetNext(string group_id, MessageMeta* info, MessageData* data,string stream)
        Error GetLast(MessageMeta* info, MessageData* data, string stream)
        Error GetById(uint64_t id, MessageMeta* info, MessageData* data, string stream)
        uint64_t GetCurrentSize(string stream, Error* err)
        uint64_t GetCurrentDatasetCount(string stream, bool include_incomplete, Error* err)
        Error SetLastReadMarker(string group_id, uint64_t value, string stream)
        Error ResetLastReadMarker(string group_id, string stream)
        Error Acknowledge(string group_id, uint64_t id, string stream)
        Error NegativeAcknowledge(string group_id, uint64_t id, uint64_t delay_ms, string stream)
        uint64_t GetLastAcknowledgedMessage(string group_id, string stream, Error* error)
        IdList GetUnacknowledgedMessages(string group_id, uint64_t from_id, uint64_t to_id, string stream, Error* error)
        string GenerateNewGroupId(Error* err)
        string GetBeamtimeMeta(Error* err)
        MessageMetas QueryMessages(string query, string stream, Error* err)
        DataSet GetNextDataset(string group_id, uint64_t min_size, string stream, Error* err)
        DataSet GetLastDataset(uint64_t min_size, string stream, Error* err)
        DataSet GetDatasetById(uint64_t id, uint64_t min_size, string stream, Error* err)
        Error RetrieveData(MessageMeta* info, MessageData* data)
        vector[StreamInfo] GetStreamList(string from_stream, StreamFilter filter, Error* err)
        void SetResendNacs(bool resend, uint64_t delay_ms, uint64_t resend_attempts)
        void InterruptCurrentOperation()

cdef extern from "asapo/asapo_consumer.h" namespace "asapo" nogil:
    cdef cppclass ConsumerFactory:
        ConsumerFactory() except +
        unique_ptr[Consumer] CreateConsumer(string server_name,string source_path,bool has_filesystem,SourceCredentials source,Error* error)


cdef extern from "asapo/asapo_consumer.h" namespace "asapo":
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
    string next_stream
