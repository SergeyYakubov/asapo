#ifndef HIDRA2_NETWORKPRODUCERPEER_H
#define HIDRA2_NETWORKPRODUCERPEER_H

#include <string>
#include <map>
#include <utility>
#include <thread>
#include <common/networking.h>
#include <system_wrappers/has_io.h>
#include <iostream>
#include <atomic>
#include <vector>

namespace hidra2 {

class NetworkProducerPeer : public HasIO {
  public:
    typedef void (*RequestHandler) (NetworkProducerPeer* self, GenericNetworkRequest* request,
                                    GenericNetworkResponse* response);
    struct RequestHandlerInformation {
        size_t request_size;
        size_t response_size;
        RequestHandler handler;
    };
  private:
    static const std::vector<RequestHandlerInformation> kRequestHandlers;
    static size_t kRequestHandlerMaxBufferSize;

    static std::atomic<uint32_t> kNetworkProducerPeerCount;


    GenericNetworkRequest* const generic_request = reinterpret_cast<GenericNetworkRequest* const>(new uint8_t[kRequestHandlerMaxBufferSize]);
    GenericNetworkResponse* const generic_response = reinterpret_cast<GenericNetworkResponse* const>(new uint8_t[kRequestHandlerMaxBufferSize]);

    uint32_t                        connection_id_;
    int                             socket_fd_;

    bool                            is_listening_ = false;
    std::unique_ptr<std::thread>    listener_thread_ = nullptr;

    void internal_receiver_thread_();
    size_t handle_generic_request_(GenericNetworkRequest* request, GenericNetworkResponse* response);

    static void handle_send_data_request_(NetworkProducerPeer* self, const SendDataRequest* request,
                                          SendDataResponse* response);
  public:
    NetworkProducerPeer& operator=(const NetworkProducerPeer&) = delete;
    NetworkProducerPeer() = default;

    NetworkProducerPeer(int socket_fd, std::string address);
    ~NetworkProducerPeer();

    static const std::vector<RequestHandlerInformation> init_request_handlers();

    void start_peer_listener();
    void stop_peer_listener();

    uint32_t GetConnectionId() const;

    //TODO make the following functions private or hide them behind an interface
    FileDescriptor CreateAndOpenFileByFileId(uint64_t file_id, Error* err);
    static bool CheckIfValidFileSize(size_t file_size);

};

}

#endif //HIDRA2_NETWORKPRODUCERPEER_H
