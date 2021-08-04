#ifndef ASAPO_CONSUMER_FABRIC_CLIENT_H
#define ASAPO_CONSUMER_FABRIC_CLIENT_H

#include <map>
#include "asapo/io/io.h"
#include <atomic>
#include <mutex>
#include "asapo/asapo_fabric/asapo_fabric.h"
#include "net_client.h"
#include "asapo/common/networking.h"

namespace asapo {

class FabricConsumerClient : public NetClient {
  public:
    explicit FabricConsumerClient();

    // modified in testings to mock system calls, otherwise do not touch
    std::unique_ptr<asapo::fabric::FabricFactory> factory__;
    std::unique_ptr<fabric::FabricClient> client__;

  private:
    std::mutex mutex_;
    std::map<std::string /* server_uri */, fabric::FabricAddress> known_addresses_;
    std::atomic<fabric::FabricMessageId> global_message_id_{0};

  public:
    Error GetData(const MessageMeta* info, const std::string& request_sender_details, MessageData* data) override;
  private:
    fabric::FabricAddress GetAddressOrConnect(const MessageMeta* info, Error* error);

    void PerformNetworkTransfer(fabric::FabricAddress address, const GenericRequestHeader* request_header,
                                GenericNetworkResponse* response, Error* err);
};
}

#endif //ASAPO_CONSUMER_FABRIC_CLIENT_H
