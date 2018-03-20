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
#include "connection.h"

namespace hidra2 {

class Connection : public HasIO {
  public:
    typedef void (* RequestHandler)(Connection* self, GenericNetworkRequest* request,
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
  public:
    static const std::vector<RequestHandlerInformation> kRequestHandlers;
    static size_t kRequestHandlerMaxBufferSize;

    static std::atomic<uint32_t> kNetworkProducerPeerImplGlobalCounter;

    Connection(SocketDescriptor socket_fd, const std::string& address);
    ~Connection();

    static const std::vector<RequestHandlerInformation> StaticInitRequestHandlerList();
    void Listen()noexcept;

    uint32_t GetId() const;
    std::string GetAddress() const;

    void ReceiveAndSaveFile(uint64_t file_id, size_t file_size, Error* err) const noexcept;

    FileDescriptor CreateAndOpenFileByFileId(uint64_t file_id, Error* err) const noexcept;
    bool CheckIfValidFileSize(size_t file_size) const noexcept;
    bool CheckIfValidNetworkOpCode(Opcode opcode) const noexcept;

  private:
    size_t HandleGenericRequest(GenericNetworkRequest* request, GenericNetworkResponse* response,
                                Error* err) noexcept;
    void HandleSendDataRequest(const SendDataRequest* request, SendDataResponse* response, Error* err) noexcept;

    void ProcessRequestFromProducer(GenericNetworkRequest* request, GenericNetworkResponse* response,
                                    Error* err) noexcept;
    void HandleRawRequestBuffer(GenericNetworkRequest* request, GenericNetworkResponse* response,
                                Error* err) noexcept;
    static void HandleSendDataRequestInternalCaller(Connection* self,
                                                    const SendDataRequest* request,
                                                    SendDataResponse* response, Error* err) noexcept;
};

}


#endif //HIDRA2_NetworkProducerPeerImpl_H
