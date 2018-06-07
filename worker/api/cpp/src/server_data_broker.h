#ifndef ASAPO_SERVER_DATA_BROKER_H
#define ASAPO_SERVER_DATA_BROKER_H

#include "worker/data_broker.h"
#include "io/io.h"
#include "http_client/http_client.h"


namespace asapo {

Error HttpCodeToWorkerError(const HttpCode& code);

class ServerDataBroker final : public asapo::DataBroker {
  public:
    explicit ServerDataBroker(const std::string& server_uri, const std::string& source_name);
    Error Connect() override;
    Error GetNext(FileInfo* info, FileData* data) override;
    void SetTimeout(uint64_t timeout_ms) override;
    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
  private:
    Error GetFileInfoFromServer(FileInfo* info, const std::string& operation);
    Error GetBrokerUri();
    void ProcessServerError(Error* err, const std::string& response, std::string* redirect_uri);
    Error ProcessRequest(std::string* response, std::string request_uri);
    std::string server_uri_;
    std::string current_broker_uri_;
    std::string source_name_;
    uint64_t timeout_ms_ = 0;
};

}

#endif //ASAPO_SERVER_DATA_BROKER_H
