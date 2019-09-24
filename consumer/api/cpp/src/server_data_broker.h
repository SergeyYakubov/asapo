#ifndef ASAPO_SERVER_DATA_BROKER_H
#define ASAPO_SERVER_DATA_BROKER_H

#include "consumer/data_broker.h"
#include "io/io.h"
#include "http_client/http_client.h"
#include "net_client.h"

namespace asapo {

Error ErrorFromServerResponce(const std::string& response, const HttpCode& code);
Error ErrorFromNoDataResponse(const std::string& response);

enum class GetImageServerOperation {
    GetNext,
    GetLast,
    GetID
};

struct RequestInfo {
    std::string host;
    std::string api;
    std::string extra_params;
    std::string body;
    bool post = false;
};


class ServerDataBroker final : public asapo::DataBroker {
  public:
    explicit ServerDataBroker(std::string server_uri, std::string source_path, SourceCredentials source);
    Error ResetLastReadMarker(std::string group_id) override;
    Error SetLastReadMarker(uint64_t value, std::string group_id) override;
    Error GetNext(FileInfo* info, std::string group_id, FileData* data) override;
    Error GetLast(FileInfo* info, std::string group_id, FileData* data) override;
    std::string GenerateNewGroupId(Error* err) override;
    std::string GetBeamtimeMeta(Error* err) override;
    uint64_t GetCurrentSize(Error* err) override;
    Error GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) override;
    void SetTimeout(uint64_t timeout_ms) override;
    FileInfos QueryImages(std::string query, Error* err) override;
    DataSet GetNextDataset(std::string group_id, Error* err) override;
    DataSet GetLastDataset(std::string group_id, Error* err) override;
    DataSet GetDatasetById(uint64_t id, std::string group_id, Error* err) override;
    Error RetrieveData(FileInfo* info, FileData* data) override;

    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
    std::unique_ptr<NetClient> net_client__;
  private:
    std::string RequestWithToken(std::string uri);
    Error GetRecordFromServer(std::string* info, std::string group_id, GetImageServerOperation op, bool dataset = false);
    Error GetRecordFromServerById(uint64_t id, std::string* info, std::string group_id, bool dataset = false);
    Error GetDataIfNeeded(FileInfo* info, FileData* data);
    Error GetBrokerUri();
    void ProcessServerError(Error* err, const std::string& response, std::string* redirect_uri);
    Error ProcessRequest(std::string* response, const RequestInfo& request);
    Error GetImageFromServer(GetImageServerOperation op, uint64_t id, std::string group_id, FileInfo* info, FileData* data);
    DataSet GetDatasetFromServer(GetImageServerOperation op, uint64_t id, std::string group_id, Error* err);
    bool DataCanBeInBuffer(const FileInfo* info);
    Error TryGetDataFromBuffer(const FileInfo* info, FileData* data);
    std::string BrokerRequestWithTimeout(RequestInfo request, Error* err);
    std::string AppendUri(std::string request_string);
    DataSet DecodeDatasetFromResponse(std::string response, Error* err);

    std::string OpToUriCmd(GetImageServerOperation op);
    std::string server_uri_;
    std::string current_broker_uri_;
    std::string source_path_;
    SourceCredentials source_credentials_;
    uint64_t timeout_ms_ = 0;
};




}

#endif //ASAPO_SERVER_DATA_BROKER_H
