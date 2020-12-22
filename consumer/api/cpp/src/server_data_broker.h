#ifndef ASAPO_SERVER_DATA_BROKER_H
#define ASAPO_SERVER_DATA_BROKER_H

#include "asapo/common/networking.h"
#include <mutex>
#include <atomic>
#include "asapo/consumer/data_broker.h"
#include "asapo/io/io.h"
#include "asapo/http_client/http_client.h"
#include "net_client.h"

namespace asapo {

enum class GetImageServerOperation {
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
    FileData data_output;
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

class ServerDataBroker final : public asapo::DataBroker {
  public:
    explicit ServerDataBroker(std::string server_uri, std::string source_path, bool has_filesystem,
                              SourceCredentials source);

    Error Acknowledge(std::string group_id, uint64_t id, std::string stream = kDefaultStream) override;
    Error NegativeAcknowledge(std::string group_id, uint64_t id, uint64_t delay_sec,
                              std::string stream = kDefaultStream) override;

    IdList GetUnacknowledgedTupleIds(std::string group_id,
                                     std::string stream,
                                     uint64_t from_id,
                                     uint64_t to_id,
                                     Error* error) override;
    IdList GetUnacknowledgedTupleIds(std::string group_id, uint64_t from_id, uint64_t to_id, Error* error) override;

    uint64_t GetLastAcknowledgedTulpeId(std::string group_id, std::string stream, Error* error) override;
    uint64_t GetLastAcknowledgedTulpeId(std::string group_id, Error* error) override;

    Error ResetLastReadMarker(std::string group_id) override;
    Error ResetLastReadMarker(std::string group_id, std::string stream) override;

    Error SetLastReadMarker(uint64_t value, std::string group_id) override;
    Error SetLastReadMarker(uint64_t value, std::string group_id, std::string stream) override;

    Error GetNext(FileInfo* info, std::string group_id, FileData* data) override;
    Error GetNext(FileInfo* info, std::string group_id, std::string stream, FileData* data) override;

    Error GetLast(FileInfo* info, FileData* data) override;
    Error GetLast(FileInfo* info, std::string stream, FileData* data) override;

    std::string GenerateNewGroupId(Error* err) override;
    std::string GetBeamtimeMeta(Error* err) override;

    uint64_t GetCurrentSize(Error* err) override;
    uint64_t GetCurrentSize(std::string stream, Error* err) override;

    Error GetById(uint64_t id, FileInfo* info, FileData* data) override;
    Error GetById(uint64_t id, FileInfo* info, std::string stream, FileData* data) override;


    void SetTimeout(uint64_t timeout_ms) override;
    void ForceNoRdma() override;

    NetworkConnectionType CurrentConnectionType() const override;

    FileInfos QueryImages(std::string query, Error* err) override;
    FileInfos QueryImages(std::string query, std::string stream, Error* err) override;

    DataSet GetNextDataset(std::string group_id, uint64_t min_size, Error* err) override;
    DataSet GetNextDataset(std::string group_id, std::string stream, uint64_t min_size, Error* err) override;

    DataSet GetLastDataset(uint64_t min_size, Error* err) override;
    DataSet GetLastDataset(std::string stream, uint64_t min_size, Error* err) override;

    DataSet GetDatasetById(uint64_t id, uint64_t min_size, Error* err) override;
    DataSet GetDatasetById(uint64_t id, std::string stream, uint64_t min_size, Error* err) override;

    Error RetrieveData(FileInfo* info, FileData* data) override;

    StreamInfos GetStreamList(std::string from, Error* err) override;
    void SetResendNacs(bool resend, uint64_t delay_sec, uint64_t resend_attempts) override;

    virtual void InterruptCurrentOperation() override;


    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
    std::unique_ptr<NetClient> net_client__;
    std::mutex net_client_mutex__; // Required for the lazy initialization of net_client
  private:
    Error GetDataFromFileTransferService(FileInfo* info, FileData* data, bool retry_with_new_token);
    Error GetDataFromFile(FileInfo* info, FileData* data);
    static const std::string kBrokerServiceName;
    static const std::string kFileTransferServiceName;
    std::string RequestWithToken(std::string uri);
    Error GetRecordFromServer(std::string* info, std::string group_id, std::string stream, GetImageServerOperation op,
                              bool dataset = false, uint64_t min_size = 0);
    Error GetRecordFromServerById(uint64_t id, std::string* info, std::string group_id, std::string stream,
                                  bool dataset = false, uint64_t min_size = 0);
    Error GetDataIfNeeded(FileInfo* info, FileData* data);
    Error DiscoverService(const std::string& service_name, std::string* uri_to_set);
    bool SwitchToGetByIdIfNoData(Error* err, const std::string& response, std::string* group_id,std::string* redirect_uri);
    bool SwitchToGetByIdIfPartialData(Error* err, const std::string& response, std::string* group_id,std::string* redirect_uri);
    Error ProcessRequest(RequestOutput* response, const RequestInfo& request, std::string* service_uri);
    Error GetImageFromServer(GetImageServerOperation op, uint64_t id, std::string group_id, std::string stream,
                             FileInfo* info, FileData* data);
    DataSet GetDatasetFromServer(GetImageServerOperation op, uint64_t id, std::string group_id, std::string stream,
                                 uint64_t min_size, Error* err);
    bool DataCanBeInBuffer(const FileInfo* info);
    Error TryGetDataFromBuffer(const FileInfo* info, FileData* data);
    Error CreateNetClientAndTryToGetFile(const FileInfo* info, FileData* data);
    Error ServiceRequestWithTimeout(const std::string& service_name, std::string* service_uri, RequestInfo request,
                                    RequestOutput* response);
    std::string BrokerRequestWithTimeout(RequestInfo request, Error* err);
    Error FtsRequestWithTimeout(FileInfo* info, FileData* data);
    Error FtsSizeRequestWithTimeout(FileInfo* info);
    Error ProcessPostRequest(const RequestInfo& request, RequestOutput* response, HttpCode* code);
    Error ProcessGetRequest(const RequestInfo& request, RequestOutput* response, HttpCode* code);

    RequestInfo PrepareRequestInfo(std::string api_url, bool dataset, uint64_t min_size);
    std::string OpToUriCmd(GetImageServerOperation op);
    Error UpdateFolderTokenIfNeeded(bool ignore_existing);
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
    RequestInfo CreateFileTransferRequest(const FileInfo* info) const;
    uint64_t resend_timout_ = 0;
    bool resend_ = false;
    uint64_t delay_sec_;
    uint64_t resend_attempts_;
    std::atomic<bool> interrupt_flag_{ false};
};

}
#endif //ASAPO_SERVER_DATA_BROKER_H
