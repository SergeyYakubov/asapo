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
                                    GenericNetworkResponse* response, Error* err);
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

    void ReceiveAndSaveFile(uint64_t file_id, size_t file_size, Error* err) const noexcept override;

    virtual FileDescriptor CreateAndOpenFileByFileId(uint64_t file_id, Error* err) const noexcept;
    virtual bool CheckIfValidFileSize(size_t file_size) const noexcept;
    virtual bool CheckIfValidNetworkOpCode(Opcode opcode) const noexcept;

  public:
    /*
     * Private functions but opened for unittest
     */
    virtual size_t HandleGenericRequest(GenericNetworkRequest* request, GenericNetworkResponse* response,
                                        Error* err) noexcept;
    virtual void HandleSendDataRequest(const SendDataRequest* request, SendDataResponse* response, Error* err) noexcept;

    //Preparation for up coming thread pool implementation
    virtual void InternalPeerReceiverThreadEntryPoint() noexcept;
    virtual void InternalPeerReceiverDoWork(GenericNetworkRequest* request, GenericNetworkResponse* response,
                                            Error* err) noexcept;
    virtual void HandleRawRequestBuffer(GenericNetworkRequest* request, GenericNetworkResponse* response,
                                        Error* err) noexcept;

  private:
    static void HandleSendDataRequestInternalCaller(NetworkProducerPeerImpl* self,
                                                    const SendDataRequest* request,
                                                    SendDataResponse* response, Error* err) noexcept;
};

}


#endif //HIDRA2_NetworkProducerPeerImpl_H
