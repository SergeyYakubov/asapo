#ifndef ASAPO_PRODUCER__PRODUCER_H
#define ASAPO_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

#include "logger/logger.h"
#include "producer/common.h"
#include "common/data_structs.h"

namespace asapo {



class Producer {
  public:
    //! Creates a new producer
    /*!
     * @return A unique_ptr to a new producer instance
     */
    static std::unique_ptr<Producer> Create(const std::string& endpoint, uint8_t n_processing_threads,
                                            asapo::RequestHandlerType type, std::string beamtime_id,
                                            Error* err);

    virtual ~Producer() = default;

    //! Sends data to the receiver
    /*!
      \param event_header - A stucture with the meta information (file name, size).
      \param data - A pointer to the data to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendData(const EventHeader& event_header, FileData data, RequestCallback callback) = 0;
    //! Sends files to the receiver
    /*!
      \param event_header - A stucture with the meta information (file name, size is ignored).
      \param file name - A full path of the file to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendFile(const EventHeader& event_header, std::string full_path, RequestCallback callback) = 0;

    //! Sends metadata for the current beamtime to the receiver
    /*!
      \param metadata - a JSON string with metadata
      \param callback - callback function
      \return Error - will be nullptr on success
    */
    virtual Error SendMetaData(const std::string& metadata, RequestCallback callback) = 0;

    //! Set internal log level
    virtual void SetLogLevel(LogLevel level) = 0;
    //! Enables/Disables logs output to stdout
    virtual void EnableLocalLog(bool enable) = 0;
    //! Enables/Disables sending logs to the central server
    virtual void EnableRemoteLog(bool enable) = 0;
    //! Set beamtime id which producer will use to send data
    virtual Error SetBeamtimeId(std::string beamtime_id) = 0;
};
}

#endif //ASAPO_PRODUCER__PRODUCER_H
