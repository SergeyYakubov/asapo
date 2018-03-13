#ifndef HIDRA2_RECEIVER_H
#define HIDRA2_RECEIVER_H

#include <string>
#include <thread>
#include <system_wrappers/has_io.h>
#include "network_producer_peer.h"
#include <list>

namespace hidra2 {

enum class ReceiverErrorType {
    kAlreadyListening,
};

class ReceiverError : public SimpleError {
  private:
    ReceiverErrorType receiver_error_type_;
  public:
    ReceiverError(const std::string& error, ReceiverErrorType receiver_error_type) : SimpleError(error,
                ErrorType::kReceiverError) {
        receiver_error_type_ = receiver_error_type;
    }

    ReceiverErrorType GetReceiverErrorType() const noexcept {
        return receiver_error_type_;
    }
};

class ReceiverErrorTemplate : public SimpleErrorTemplate {
  protected:
    ReceiverErrorType receiver_error_type_;
  public:
    ReceiverErrorTemplate(const std::string& error, ReceiverErrorType receiver_error_type) : SimpleErrorTemplate(error,
                ErrorType::kIOError) {
        receiver_error_type_ = receiver_error_type;
    }

    inline ReceiverErrorType GetReceiverErrorType() const noexcept {
        return receiver_error_type_;
    }

    inline Error Generate() const noexcept override {
        return Error(new ReceiverError(error_, receiver_error_type_));
    }

    inline bool operator == (const Error& rhs) const override {
        return SimpleErrorTemplate::operator==(rhs)
               && GetReceiverErrorType() == ((ReceiverError*)rhs.get())->GetReceiverErrorType();
    }
};

namespace ReceiverErrorTemplates {
auto const kAlreadyListening = ReceiverErrorTemplate{"Receiver is already listening", ReceiverErrorType::kAlreadyListening};
};

class Receiver : public HasIO {
    friend NetworkProducerPeer;
  private:
    bool listener_running_ = false;
    FileDescriptor listener_fd_ = -1;
    std::unique_ptr<std::thread> accept_thread_ = nullptr;

    void AcceptThreadLogic();
    std::list<std::unique_ptr<NetworkProducerPeer>> peer_list_;
    std::unique_ptr<NetworkProducerPeer> on_new_peer_(int peer_socket_fd, std::string address);

  public:
    static const int kMaxUnacceptedConnectionsBacklog;//TODO: Read from config

    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;

    Receiver() = default;

    void StartListener(std::string listener_address, Error* err);
    void StopListener(Error* err);
};

}

#endif //HIDRA2_RECEIVER_H
