#ifndef ASAPO_PRODUCER__PRODUCER_H
#define ASAPO_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

#include "producer_error.h"
#include "logger/logger.h"

namespace asapo {

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
      \return Error - nullptr on success
    */
    virtual Error ConnectToReceiver(const std::string& receiver_address) = 0;
    //! Sends data to the receiver
    /*!
      \param file_id - The id of the file. An error will be returned if this file id already exists on the receiver.
      \param data - A pointer to the data to send
      \param file_size - The size of the data.
      \return Error - Will be nullptr on success
    */
    virtual Error Send(uint64_t file_id, const void* data, size_t file_size) = 0;
    //! Set internal log level
    virtual void SetLogLevel(LogLevel level) = 0;
    //! Enables/Disables logs output to stdout
    virtual void EnableLocalLog(bool enable) = 0;
    //! Enables/Disables sending logs to the central server
    virtual void EnableRemoteLog(bool enable) = 0;
};
}

#endif //ASAPO_PRODUCER__PRODUCER_H
