#ifndef HIDRA2_NetworkProducerPeerImpl_H
#define HIDRA2_NetworkProducerPeerImpl_H

#include <string>
#include <map>
#include <utility>
#include <thread>
#include <common/networking.h>
#include <system_wrappers/has_io.h>
#include <iostream>
#include <atomic>
#include <vector>
#include "network_producer_peer.h"

namespace hidra2 {

class NetworkProducerPeerImpl : public NetworkProducerPeer, public HasIO {
  public:
    typedef void (* RequestHandler)(NetworkProducerPeerImpl* self, GenericNetworkRequest* request,
                                    GenericNetworkResponse* response);
    struct RequestHandlerInformation {
        size_t request_size;
        size_t response_size;
        RequestHandler handler;
    };
  private:
    uint32_t connection_id_;
    std::string address_;
    int socket_fd_;

    bool is_listening_ = false;
    std::unique_ptr<std::thread> listener_thread_ = nullptr;

    //Preparation for up coming thread pool implementation
    void InternalPeerReceiverThreadEntryPoint();
    void InternalPeerReceiverDoWork(GenericNetworkRequest* request, GenericNetworkResponse* response, Error* err);
    void HandleRawRequestBuffer(GenericNetworkRequest* request, GenericNetworkResponse* response, Error* err);
  protected:
    void ReceiveAndSaveFile(uint64_t file_id, size_t file_size, Error* err) override;
  public:
    static const std::vector<RequestHandlerInformation> kRequestHandlers;
    static size_t kRequestHandlerMaxBufferSize;

    static std::atomic<uint32_t> kNetworkProducerPeerImplGlobalCounter;

    NetworkProducerPeerImpl(SocketDescriptor socket_fd, const std::string& address);
    ~NetworkProducerPeerImpl() override;

    static const std::vector<RequestHandlerInformation> StaticInitRequestHandlerList();
    void StartPeerListener() override;
    void StopPeerListener() override;

    uint32_t GetConnectionId() const override;
    std::string GetAddress() const override;

    FileDescriptor CreateAndOpenFileByFileId(uint64_t file_id, Error* err);
    static bool CheckIfValidFileSize(size_t file_size);
    static bool CheckIfValidNetworkOpCode(Opcode opcode);

  public:
    /*
     * Private functions but opened for unittest
     */
    size_t HandleGenericRequest(GenericNetworkRequest* request, GenericNetworkResponse* response, Error* err);
    static void HandleSendDataRequest(NetworkProducerPeerImpl* self,
                                      const SendDataRequest* request,
                                      SendDataResponse* response);

};

}


#endif //HIDRA2_NetworkProducerPeerImpl_H
