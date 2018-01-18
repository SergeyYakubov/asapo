#ifndef HIDRA2_PRODUCER__PRODUCER_H
#define HIDRA2_PRODUCER__PRODUCER_H

#include <string>
#include <memory>
#include <common/networking.h>
#include <system_wrappers/io.h>
#include <system_wrappers/has_io.h>

namespace hidra2 {
enum ProducerError {
    PRODUCER_ERROR__NO_ERROR,
    PRODUCER_ERROR__ALREADY_CONNECTED,
    PRODUCER_ERROR__CONNECTION_NOT_READY,
    PRODUCER_ERROR__FAILED_TO_CONNECT_TO_SERVER,
    PRODUCER_ERROR__INVALID_ADDRESS_FORMAT,
    PRODUCER_ERROR__UNEXPECTED_IO_ERROR,
};

enum ProducerStatus {
    PRODUCER_STATUS__DISCONNECTED,
    PRODUCER_STATUS__CONNECTING,
    PRODUCER_STATUS__CONNECTED,
    PRODUCER_STATUS__SENDING,
    PRODUCER_STATUS__ERROR,
};

class Producer : public HasIO {
  public:
    static std::unique_ptr<Producer> create();

    //virtual ~Producer() = 0;

    virtual uint64_t GetVersion() const = 0;
    virtual ProducerStatus GetStatus() const = 0;

    virtual ProducerError ConnectToReceiver(const std::string& receiver_address) = 0;
    virtual ProducerError Send(uint64_t file_id, void* data, uint64_t file_size) = 0;
};
}

#endif //HIDRA2_PRODUCER__PRODUCER_H
