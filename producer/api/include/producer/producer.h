#ifndef ASAPO_PRODUCER__PRODUCER_H
#define ASAPO_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

#include "logger/logger.h"
#include "producer/common.h"

namespace asapo {


class Producer {
  public:
    //! Creates a new producer
    /*!
     * @return A unique_ptr to a new producer instance
     */
    static std::unique_ptr<Producer> Create(const std::string& endpoint, uint8_t n_processing_threads, asapo::RequestHandlerType type,
                                            Error* err);

    virtual ~Producer() = default;

    //! Sends data to the receiver
    /*!
      \param file_id - The id of the file. An error will be returned if this file id already exists on the receiver.
      \param data - A pointer to the data to send
      \param file_size - The size of the data.
      \return Error - Will be nullptr on success
    */
    virtual Error Send(uint64_t file_id, const void* data, size_t file_size,std::string file_name, RequestCallback callback) = 0;
    //! Set internal log level
    virtual void SetLogLevel(LogLevel level) = 0;
    //! Enables/Disables logs output to stdout
    virtual void EnableLocalLog(bool enable) = 0;
    //! Enables/Disables sending logs to the central server
    virtual void EnableRemoteLog(bool enable) = 0;
};
}

#endif //ASAPO_PRODUCER__PRODUCER_H
