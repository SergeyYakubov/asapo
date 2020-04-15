#ifndef ASAPO_CONSUMER_FABRIC_CLIENT_H
#define ASAPO_CONSUMER_FABRIC_CLIENT_H

#include <map>
#include <io/io.h>
#include <atomic>
#include <mutex>
#include "asapo_fabric/asapo_fabric.h"
#include "net_client.h"

namespace asapo {

class FabricClient : NetClient {
  public:
    explicit FabricClient();

    // modified in testings to mock system calls, otherwise do not touch
    std::unique_ptr<asapo::fabric::FabricFactory> factory__;
    std::unique_ptr<IO> io__;
    std::unique_ptr<fabric::FabricClient> client__;

  private:
    std::mutex mutex_;
    std::map<std::string /* server_uri */, fabric::FabricAddress> known_addresses_;
    std::atomic<fabric::FabricMessageId> global_message_id_{0};

  public:
    Error GetData(const FileInfo* info, FileData* data) override;
  private:
    fabric::FabricAddress GetAddressOrConnect(const FileInfo* info, Error* error);

};
}

#endif //ASAPO_CONSUMER_FABRIC_CLIENT_H
