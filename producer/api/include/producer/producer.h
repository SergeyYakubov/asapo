#ifndef HIDRA2_PRODUCER__PRODUCER_H
#define HIDRA2_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

namespace hidra2 {
enum class ProducerError {
    kUnknownError,
    kNoError,
    kUnknownServerError,
    kAlreadyConnected,
    kConnectionNotReady,
    kInvalidAddressFormat,
    kUnexpectedIOError,
    kFileIdAlreadyInUse,
    kFileTooLarge,
    kConnectionRefused,
};

enum class ProducerStatus {
    kDisconnected,
    kConnected,
};

class Producer {
  public:
    //! Creates a new producer
    /*!
     * @return A unique_ptr to a new producer instance
     */
    static std::unique_ptr<Producer> Create();

    virtual ~Producer() = default;

    /*!
     * @return The version of the producer
     */
    virtual uint64_t GetVersion() const = 0;

    /*!
     * @return The current status of the producer
     */
    virtual ProducerStatus GetStatus() const = 0;

    //! Connects to a receiver
    /*!
      \param receiver_address - The address of the receiver. E.g. 127.0.0.1:4200
      \return ProducerError - Will be ProducerError::kNoError on success
    */
    virtual ProducerError ConnectToReceiver(const std::string& receiver_address) = 0;
    //! Sends data to the receiver
    /*!
      \param file_id - The id of the file. An error will be returned if this file id already exists on the receiver.
      \param data - A pointer to the data to send
      \param file_size - The size of the data.
      \return ProducerError - Will be ProducerError::kNoError on success
    */
    virtual ProducerError Send(uint64_t file_id, const void* data, size_t file_size) = 0;
};
}

#endif //HIDRA2_PRODUCER__PRODUCER_H
