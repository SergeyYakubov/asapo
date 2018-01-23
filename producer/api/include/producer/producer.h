#ifndef HIDRA2_PRODUCER__PRODUCER_H
#define HIDRA2_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

namespace hidra2 {
enum class ProducerError {
    kNoError,
    kAlreadyConnected,
    kConnectionNotReady,
    kInvalidAddressFormat,
    kUnexpectedIOError,
    kFileIdAlreadyInUse,
    kFileTooLarge,
    kUnknownServerError,
    kUnknownError,
};

enum class ProducerStatus {
    kDisconnected,
    kConnecting,
    kConnected,
    kSending,
};

class Producer {
  public:
    static std::unique_ptr<Producer> create();

    //virtual ~Producer() = 0;

    virtual uint64_t GetVersion() const = 0;
    virtual ProducerStatus GetStatus() const = 0;

    virtual ProducerError ConnectToReceiver(const std::string& receiver_address) = 0;
    virtual ProducerError Send(uint64_t file_id, void* data, size_t file_size) = 0;
};
}

#endif //HIDRA2_PRODUCER__PRODUCER_H
