#ifndef ASAPO_SERVER_DATA_BROKER_H
#define ASAPO_SERVER_DATA_BROKER_H

#include <common/networking.h>
#include "consumer/data_broker.h"
#include "io/io.h"
#include "http_client/http_client.h"
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


class ServerDataBroker final : public asapo::DataBroker {
  public:
    explicit ServerDataBroker(std::string server_uri, std::string source_path, bool has_filesystem,
                              SourceCredentials source, NetworkConnectionType networkType);
    Error Acknowledge(std::string group_id, uint64_t id, std::string substream = kDefaultSubstream) override;

    IdList GetUnacknowledgedTupleIds(std::string group_id, std::string substream, uint64_t from_id, uint64_t to_id, Error* error) override;
    IdList GetUnacknowledgedTupleIds(std::string group_id, uint64_t from_id, uint64_t to_id, Error* error) override;

    uint64_t GetLastAcknowledgedTulpeId(std::string group_id, std::string substream, Error* error) override;
    uint64_t GetLastAcknowledgedTulpeId(std::string group_id, Error* error) override;
    Error ResetLastReadMarker(std::string group_id) override;
    Error ResetLastReadMarker(std::string group_id, std::string substream) override;

    Error SetLastReadMarker(uint64_t value, std::string group_id) override;
    Error SetLastReadMarker(uint64_t value, std::string group_id, std::string substream) override;

    Error GetNext(FileInfo* info, std::string group_id, FileData* data) override;
    Error GetNext(FileInfo* info, std::string group_id, std::string substream, FileData* data) override;

    Error GetLast(FileInfo* info, std::string group_id, FileData* data) override;
    Error GetLast(FileInfo* info, std::string group_id, std::string substream, FileData* data) override;

    std::string GenerateNewGroupId(Error* err) override;
    std::string GetBeamtimeMeta(Error* err) override;

    uint64_t GetCurrentSize(Error* err) override;
    uint64_t GetCurrentSize(std::string substream, Error* err) override;


    Error GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) override;
    Error GetById(uint64_t id, FileInfo* info, std::string group_id, std::string substream, FileData* data) override;

    void SetTimeout(uint64_t timeout_ms) override;
    FileInfos QueryImages(std::string query, Error* err) override;
    FileInfos QueryImages(std::string query, std::string substream, Error* err) override;

    DataSet GetNextDataset(std::string group_id, Error* err) override;
    DataSet GetNextDataset(std::string group_id, std::string substream, Error* err) override;

    DataSet GetLastDataset(std::string group_id, Error* err) override;
    DataSet GetLastDataset(std::string group_id, std::string substream, Error* err) override;

    DataSet GetDatasetById(uint64_t id, std::string group_id, Error* err) override;
    DataSet GetDatasetById(uint64_t id, std::string group_id, std::string substream, Error* err) override;

    Error RetrieveData(FileInfo* info, FileData* data) override;

    std::vector<std::string> GetSubstreamList(Error* err) override;

    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
    std::unique_ptr<NetClient> net_client__;
  private:
    Error GetDataFromFileTransferService(const FileInfo* info, FileData* data, bool retry_with_new_token);
    Error GetDataFromFile(FileInfo* info, FileData* data);
    static const std::string kBrokerServiceName;
    static const std::string kFileTransferServiceName;
    std::string RequestWithToken(std::string uri);
    Error GetRecordFromServer(std::string* info, std::string group_id, std::string substream, GetImageServerOperation op,
                              bool dataset = false);
    Error GetRecordFromServerById(uint64_t id, std::string* info, std::string group_id, std::string substream,
                                  bool dataset = false);
    Error GetDataIfNeeded(FileInfo* info, FileData* data);
    Error DiscoverService(const std::string& service_name, std::string* uri_to_set);
    bool SwitchToGetByIdIfNoData(Error* err, const std::string& response, std::string* redirect_uri);
    Error ProcessRequest(RequestOutput* response, const RequestInfo& request, std::string* service_uri);
    Error GetImageFromServer(GetImageServerOperation op, uint64_t id, std::string group_id, std::string substream,
                             FileInfo* info, FileData* data);
    DataSet GetDatasetFromServer(GetImageServerOperation op, uint64_t id, std::string group_id, std::string substream,
                                 Error* err);
    bool DataCanBeInBuffer(const FileInfo* info);
    Error TryGetDataFromBuffer(const FileInfo* info, FileData* data);
    Error ServiceRequestWithTimeout(const std::string& service_name, std::string* service_uri, RequestInfo request,
                                    RequestOutput* response);
    std::string BrokerRequestWithTimeout(RequestInfo request, Error* err);
    Error FtsRequestWithTimeout(const FileInfo* info, FileData* data);
    Error ProcessPostRequest(const RequestInfo& request, RequestOutput* response, HttpCode* code);
    Error ProcessGetRequest(const RequestInfo& request, RequestOutput* response, HttpCode* code);

    DataSet DecodeDatasetFromResponse(std::string response, Error* err);
    RequestInfo PrepareRequestInfo(std::string api_url, bool dataset);
    std::string OpToUriCmd(GetImageServerOperation op);
    Error UpdateFolderTokenIfNeeded(bool ignore_existing);
    std::string endpoint_;
    std::string current_broker_uri_;
    std::string current_fts_uri_;
    std::string source_path_;
    bool has_filesystem_;
    SourceCredentials source_credentials_;
    uint64_t timeout_ms_ = 0;
    std::string folder_token_;
    RequestInfo CreateFolderTokenRequest() const;
    RequestInfo CreateFileTransferRequest(const FileInfo* info) const;
};




}

#endif //ASAPO_SERVER_DATA_BROKER_H
