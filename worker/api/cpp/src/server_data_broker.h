#ifndef ASAPO_SERVER_DATA_BROKER_H
#define ASAPO_SERVER_DATA_BROKER_H

#include "worker/data_broker.h"
#include "io/io.h"
#include "http_client/http_client.h"
#include "net_client.h"

namespace asapo {

Error HttpCodeToWorkerError(const HttpCode& code);

enum class GetImageServerOperation {
    GetNext,
    GetLast,
    GetID
};

class ServerDataBroker final : public asapo::DataBroker {
  public:
    explicit ServerDataBroker(std::string server_uri, std::string source_path, std::string source_name, std::string token);
    Error Connect() override;
    Error ResetCounter(std::string group_id) override;
    Error GetNext(FileInfo* info, std::string group_id, FileData* data) override;
    Error GetLast(FileInfo* info, std::string group_id, FileData* data) override;
    std::string GenerateNewGroupId(Error* err) override;
    std::string GetBeamtimeMeta(Error* err) override;
    uint64_t GetNDataSets(Error* err) override;
    Error GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) override;
    void SetTimeout(uint64_t timeout_ms) override;
    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
    std::unique_ptr<NetClient> net_client__;
  private:
    std::string RequestWithToken(std::string uri);
    Error GetFileInfoFromServer(FileInfo* info, std::string group_id, GetImageServerOperation op);
    Error GetFileInfoFromServerById(uint64_t id, FileInfo* info, std::string group_id);
    Error GetDataIfNeeded(FileInfo* info, FileData* data);
    Error GetBrokerUri();
    void ProcessServerError(Error* err, const std::string& response, std::string* redirect_uri);
    Error ProcessRequest(std::string* response, std::string request_uri, std::string extra_params, bool post);
    Error GetImageFromServer(GetImageServerOperation op, uint64_t id, std::string group_id, FileInfo* info, FileData* data);
    bool DataCanBeInBuffer(const FileInfo* info);
    Error TryGetDataFromBuffer(const FileInfo* info, FileData* data);
    std::string BrokerRequestWithTimeout(std::string request_string, std::string extra_params, bool post_request,
                                         Error* err);
    std::string OpToUriCmd(GetImageServerOperation op);
    std::string server_uri_;
    std::string current_broker_uri_;
    std::string source_path_;
    std::string source_name_;
    std::string token_;
    uint64_t timeout_ms_ = 0;
};




}

#endif //ASAPO_SERVER_DATA_BROKER_H
