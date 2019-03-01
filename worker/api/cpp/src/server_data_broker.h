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
    GetLast
};

class ServerDataBroker final : public asapo::DataBroker {
  public:
    explicit ServerDataBroker(std::string server_uri, std::string source_path, std::string source_name, std::string token);
    Error Connect() override;
    Error GetNext(FileInfo* info, FileData* data) override;
    Error GetLast(FileInfo* info, FileData* data) override;

    void SetTimeout(uint64_t timeout_ms) override;
    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
    std::unique_ptr<NetClient> net_client__;
  private:
    std::string RequestWithToken(std::string uri);
    Error GetFileInfoFromServer(FileInfo* info, GetImageServerOperation op);
    Error GetDataIfNeeded(FileInfo* info, FileData* data);
    Error GetBrokerUri();
    void ProcessServerError(Error* err, const std::string& response, std::string* redirect_uri);
    Error ProcessRequest(std::string* response, std::string request_uri);
    Error GetImageFromServer(GetImageServerOperation op, FileInfo* info, FileData* data);
    bool DataCanBeInBuffer(const FileInfo* info);
    Error TryGetDataFromBuffer(const FileInfo* info, FileData* data);
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
