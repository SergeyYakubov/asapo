#ifndef HIDRA2_PRODUCER__PRODUCER_H
#define HIDRA2_PRODUCER__PRODUCER_H

#include <string>
#include <memory>
#include <common/networking.h>
#include <system_wrappers/io.h>
#include <system_wrappers/has_io.h>

namespace hidra2 {
enum ProducerError {
    PRODUCER_ERROR__OK,
    PRODUCER_ERROR__ALREADY_CONNECTED,
    PRODUCER_ERROR__CONNECTION_NOT_READY,
    PRODUCER_ERROR__SENDING_CHUNK_FAILED,
    PRODUCER_ERROR__RECEIVING_SERVER_RESPONSE_FAILED,
    PRODUCER_ERROR__SERVER_REPORTED_AN_ERROR,
};

enum ProducerStatus {
    PRODUCER_STATUS__DISCONNECTED,
    PRODUCER_STATUS__CONNECTING,
    PRODUCER_STATUS__CONNECTED,
    PRODUCER_STATUS__SENDING,
    PRODUCER_STATUS__ERROR,
};

struct FileChunk {
    void *ptr;
    uint64_t start_byte;
    uint64_t chunk_size;
};

class Producer : public HasIO {
  public:
    static std::unique_ptr<Producer> create();

    //virtual ~Producer() = 0;

    virtual uint64_t get_version() const = 0;
    virtual ProducerStatus get_status() const = 0;

    virtual ProducerError connect_to_receiver(std::string receiver_address) = 0;
    virtual ProducerError send(std::string filename, void* data, uint64_t file_size) = 0;
};
}

#endif //HIDRA2_PRODUCER__PRODUCER_H
