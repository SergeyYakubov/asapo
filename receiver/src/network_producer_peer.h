#ifndef HIDRA2_NETWORKPRODUCERPEER_H
#define HIDRA2_NETWORKPRODUCERPEER_H

#include <string>
#include <utility>
#include <thread>
#include <common/networking.h>
#include <system_wrappers/has_io.h>
#include <iostream>
#include <atomic>
#include <vector>

namespace hidra2 {

class NetworkProducerPeer : HasIO {
  public:
    typedef void (*RequestHandler) (NetworkProducerPeer* self, GenericNetworkRequest* request, GenericNetworkResponse* response);
    struct RequestHandlerInformation {
        size_t request_size;
        size_t response_size;
        RequestHandler handler;
    };
  private:

    static const std::vector<RequestHandlerInformation> request_handlers;
    static std::atomic<uint32_t> kNetworkProducerPeerCount;

    uint32_t        connection_id_;
    int             socket_fd_;
    std::string     address_;

    std::thread*    receiver_thread_ = nullptr;

    void internal_receiver_thread_();
    size_t handle_generic_request_(GenericNetworkRequest* request, GenericNetworkResponse* response);

    static void handle_hello_request_(NetworkProducerPeer* self, const HelloRequest* request, HelloResponse* response);
    static void handle_prepare_send_data_request_(NetworkProducerPeer* self, const PrepareSendDataRequest* request, PrepareSendDataResponse* response);
    static void handle_send_data_chunk_request_(NetworkProducerPeer* self, const SendDataChunkRequest* request, SendDataChunkResponse* response);

 public:
    NetworkProducerPeer(int socket_fd, std::string aHelloRequestddress);

    static const std::vector<RequestHandlerInformation> init_request_handlers();

    uint32_t connection_id() const;

    void start_peer_receiver();
    void stop_peer_receiver();

    const std::string& address() const;
    void disconnect();
};

}

#endif //HIDRA2_NETWORKPRODUCERPEER_H
