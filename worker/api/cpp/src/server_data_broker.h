#ifndef HIDRA2_SERVER_DATA_BROKER_H
#define HIDRA2_SERVER_DATA_BROKER_H

#include "worker/data_broker.h"
#include "io/io.h"
#include "http_client/http_client.h"


namespace hidra2 {

Error HttpCodeToWorkerError(const HttpCode& code);

class ServerDataBroker final : public hidra2::DataBroker {
  public:
    explicit ServerDataBroker(const std::string& server_uri, const std::string& source_name);
    Error Connect() override;
    Error GetNext(FileInfo* info, FileData* data) override;
    std::unique_ptr<IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<HttpClient> httpclient__;
  private:
    Error GetFileInfoFromServer(FileInfo* info, const std::string& operation);
    std::string server_uri_;
    std::string source_name_;
};

}

#endif //HIDRA2_SERVER_DATA_BROKER_H
