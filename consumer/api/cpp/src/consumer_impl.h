#ifndef ASAPO_CONSUMER_IMPL_H
#define ASAPO_CONSUMER_IMPL_H

#include "asapo/common/networking.h"
#include <mutex>
#include <atomic>
#include "asapo/consumer/consumer.h"
#include "asapo/io/io.h"
#include "asapo/http_client/http_client.h"
#include "net_client.h"

namespace asapo {

enum class GetMessageServerOperation {
    GetNext,
    GetLast,
    GetID
};

enum class OutputDataMode {
    string,
    array,
    file
};

struct RequestInfo {
    std::string host;
    std::string api;
    std::string extra_params;
    std::string body;
    std::string cookie;
    OutputDataMode output_mode = OutputDataMode::string;
    bool post = false;
};

struct RequestOutput {
    std::string string_output;
    MessageData data_output;
    uint64_t data_output_size;
    const char* to_string() const {
        if (!data_output) {
            return string_output.c_str();
        } else {
            return reinterpret_cast<char const*>(data_output.get()) ;
        }
    }
};

Error ProcessRequestResponce(const Error& server_err, const RequestOutput* response, const HttpCode& code);
Error ConsumerErrorFromNoDataResponse(const std::string& response);
Error ConsumerErrorFromPartialDataResponse(const std::string& response);
DataSet DecodeDatasetFromResponse(std::string response, Error* err);

class ConsumerImpl final : public asapo::Consumer {
  public:
    explicit ConsumerImpl(std::string server_uri, std::string source_path, bool has_filesystem,
                          SourceCredentials source);

    Error Acknowledge(std::string group_id, uint64_t id, std::string) override;
    Error NegativeAcknowledge(std::string group_id, uint64_t id, uint64_t delay_ms,
                              std::string stream) override;

    IdList GetUnacknowledgedMessages(std::string group_id,
                                     uint64_t from_id,
                                     uint64_t to_id,
                                     std::string stream,
                                     Error* error) override;

    uint64_t GetLastAcknowledgedMessage(std::string group_id, std::string stream, Error* error) override;

    Error ResetLastReadMarker(std::string group_id, std::string stream) override;

    Error SetLastReadMarker(std::string group_id, uint64_t value, std::string stream) override;

    Error GetNext(std::string group_id, MessageMeta* info, MessageData* data, std::string stream) override;

    Error GetLast(MessageMeta* info, MessageData* data, std::string stream) override;

    std::string GenerateNewGroupId(Error* err) override;
    std::string GetBeamtimeMeta(Error* err) override;

    uint64_t GetCurrentSize(std::string stream, Error* err) override;
    uint64_t GetCurrentDatasetCount(std::string stream, bool include_incomplete, Error* err) override;

    Error GetById(uint64_t id, MessageMeta* info, MessageData* data, std::string stream) override;

    Error GetVersionInfo(std::string* client_info,std::string* server_info, bool* supported) override;
    Error DeleteStream(std::string stream, DeleteStreamOptions options) override;
    void SetTimeout(uint64_t timeout_ms) override;
    void ForceNoRdma() override;

    NetworkConnectionType CurrentConnectionType() const override;

    MessageMetas QueryMessages(std::string query, std::string stream, Error* err) override;

    DataSet GetNextDataset(std::string group_id, uint64_t min_size, std::string stream, Error* err) override;

    DataSet GetLastDataset(uint64_t min_size, std::string stream, Error* err) override;

    DataSet GetDatasetById(uint64_t id, uint64_t min_size, std::string stream, Error* err) override;

    Error RetrieveData(MessageMeta* info, MessageData* data) override;

    StreamInfos GetStreamList(std::string from, StreamFilter filter, Error* err) override;
    void SetResendNacs(bool resend, uint64_t delay_ms, uint64_t resend_attempts) override;

    virtual void InterruptCurrentOperation() override;


    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
    std::unique_ptr<NetClient> net_client__;
    std::mutex net_client_mutex__; // Required for the lazy initialization of net_client
  private:
    Error ProcessDiscoverServiceResult(Error err, std::string* uri_to_set);
    Error GetDataFromFileTransferService(MessageMeta* info, MessageData* data, bool retry_with_new_token);
    Error GetDataFromFile(MessageMeta* info, MessageData* data);
    static const std::string kBrokerServiceName;
    static const std::string kFileTransferServiceName;
    std::string RequestWithToken(std::string uri);
    Error GetRecordFromServer(std::string* info, std::string group_id, std::string stream, GetMessageServerOperation op,
                              bool dataset = false, uint64_t min_size = 0);
    Error GetRecordFromServerById(uint64_t id, std::string* info, std::string group_id, std::string stream,
                                  bool dataset = false, uint64_t min_size = 0);
    Error GetDataIfNeeded(MessageMeta* info, MessageData* data);
    Error DiscoverService(const std::string& service_name, std::string* uri_to_set);
    bool SwitchToGetByIdIfNoData(Error* err, const std::string& response, std::string* group_id,std::string* redirect_uri);
    bool SwitchToGetByIdIfPartialData(Error* err, const std::string& response, std::string* group_id,std::string* redirect_uri);
    Error ProcessRequest(RequestOutput* response, const RequestInfo& request, std::string* service_uri);
    Error GetMessageFromServer(GetMessageServerOperation op, uint64_t id, std::string group_id, std::string stream,
                             MessageMeta* info, MessageData* data);
    DataSet GetDatasetFromServer(GetMessageServerOperation op, uint64_t id, std::string group_id, std::string stream,
                                 uint64_t min_size, Error* err);
    bool DataCanBeInBuffer(const MessageMeta* info);
    Error TryGetDataFromBuffer(const MessageMeta* info, MessageData* data);
    Error CreateNetClientAndTryToGetFile(const MessageMeta* info, MessageData* data);
    Error ServiceRequestWithTimeout(const std::string& service_name, std::string* service_uri, RequestInfo request,
                                    RequestOutput* response);
    std::string BrokerRequestWithTimeout(RequestInfo request, Error* err);
    Error FtsRequestWithTimeout(MessageMeta* info, MessageData* data);
    Error FtsSizeRequestWithTimeout(MessageMeta* info);
    Error ProcessPostRequest(const RequestInfo& request, RequestOutput* response, HttpCode* code);
    Error ProcessGetRequest(const RequestInfo& request, RequestOutput* response, HttpCode* code);
    RequestInfo PrepareRequestInfo(std::string api_url, bool dataset, uint64_t min_size);
    std::string OpToUriCmd(GetMessageServerOperation op);
    Error UpdateFolderTokenIfNeeded(bool ignore_existing);

    uint64_t GetCurrentCount(std::string stream, const RequestInfo& ri, Error* err);
    RequestInfo GetStreamListRequest(const std::string &from, const StreamFilter &filter) const;
    Error GetServerVersionInfo(std::string* server_info, bool* supported) ;

    std::string endpoint_;
    std::string current_broker_uri_;
    std::string current_fts_uri_;
    std::string source_path_;
    bool has_filesystem_;
    SourceCredentials source_credentials_;
    uint64_t timeout_ms_ = 0;
    bool should_try_rdma_first_ = true;
    NetworkConnectionType current_connection_type_ = NetworkConnectionType::kUndefined;
    std::string folder_token_;
    RequestInfo CreateFolderTokenRequest() const;
    RequestInfo CreateFileTransferRequest(const MessageMeta* info) const;
    uint64_t resend_timout_ = 0;
    bool resend_ = false;
    uint64_t delay_ms_;
    uint64_t resend_attempts_;
    std::atomic<bool> interrupt_flag_{ false};

  RequestInfo GetSizeRequestForSingleMessagesStream(std::string &stream) const;
  RequestInfo GetSizeRequestForDatasetStream(std::string &stream, bool include_incomplete) const;
  uint64_t ParseGetCurrentCountResponce(Error* err, const std::string &responce) const;
  RequestInfo GetDiscoveryRequest(const std::string &service_name) const;
  RequestInfo GetVersionRequest() const;
  RequestInfo GetDeleteStreamRequest(std::string stream, DeleteStreamOptions options) const;
};

}
#endif //ASAPO_CONSUMER_IMPL_H
