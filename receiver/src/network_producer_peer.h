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
#include "file_refernce_handler.h"

namespace hidra2 {

class NetworkProducerPeer : HasIO {
  public:
    typedef void (*RequestHandler) (NetworkProducerPeer* self, GenericNetworkRequest* request,
                                    GenericNetworkResponse* response);
    struct RequestHandlerInformation {
        size_t request_size;
        size_t response_size;
        RequestHandler handler;
    };
  private:
    /** Must be as large as the largest request type (not including the data) */
    static const size_t kGenericBufferSize;

    static FileReferenceHandler file_reference_handler;
    static const std::vector<RequestHandlerInformation> kRequestHandlers;
    static std::atomic<uint32_t> kNetworkProducerPeerCount;

    uint32_t        connection_id_;
    int             socket_fd_;
    std::string     address_;

    bool            got_hello_ = false;

    bool            is_listening_ = false;
    std::thread*    listener_thread_ = nullptr;

    void internal_receiver_thread_();
    size_t handle_generic_request_(GenericNetworkRequest* request, GenericNetworkResponse* response);

    static void handle_hello_request_(NetworkProducerPeer* self, const HelloRequest* request, HelloResponse* response);
    static void handle_prepare_send_data_request_(NetworkProducerPeer* self, const PrepareSendDataRequest* request,
                                                  PrepareSendDataResponse* response);
    static void handle_send_data_chunk_request_(NetworkProducerPeer* self, const SendDataChunkRequest* request,
                                                SendDataChunkResponse* response);

  public:
    NetworkProducerPeer& operator=(const NetworkProducerPeer&) = delete;
    NetworkProducerPeer() = default;

    NetworkProducerPeer(int socket_fd, std::string address);
    ~NetworkProducerPeer();

    static const std::vector<RequestHandlerInformation> init_request_handlers();


    void start_peer_listener();
    void stop_peer_listener();

    uint32_t connection_id() const;
    const std::string& address() const;
    bool is_listening() const;
};

}

#endif //HIDRA2_NETWORKPRODUCERPEER_H
