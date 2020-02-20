#ifndef ASAPO_PRODUCER__PRODUCER_H
#define ASAPO_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

#include "logger/logger.h"
#include "producer/common.h"
#include "common/data_structs.h"

namespace asapo {


/** @ingroup producer */
class Producer {
  public:
    //! Creates a new producer
    /*!
     * @return A unique_ptr to a new producer instance
     */
    static std::unique_ptr<Producer> Create(const std::string& endpoint, uint8_t n_processing_threads,
                                            asapo::RequestHandlerType type, SourceCredentials source_cred,
                                            uint64_t timeout_sec,
                                            Error* err);

    virtual ~Producer() = default;

    //! Sends data to the receiver
    /*!
      \param event_header - A stucture with the meta information (file name, size, a string with user metadata (JSON format)).
      \param data - A pointer to the data to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendData(const EventHeader& event_header, FileData data, uint64_t ingest_mode,
                           RequestCallback callback) = 0;


    //! Sends data to the receiver - same as SendData - memory should not be freed until send is finished
    //! used e.g. for Python bindings
    virtual Error SendData__(const EventHeader& event_header, void* data, uint64_t ingest_mode,
                             RequestCallback callback) = 0;

    //! Sends data to the receiver
    /*!
      \param event_header - A stucture with the meta information (file name, size, a string with user metadata (JSON format)).
      \param data - A pointer to the data to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendData(const EventHeader& event_header, std::string substream, FileData data, uint64_t ingest_mode,
                           RequestCallback callback) = 0;


    //! Sends data to the receiver - same as SendData - memory should not be freed until send is finished
    //! used e.g. for Python bindings
    virtual Error SendData__(const EventHeader& event_header, std::string substream, void* data, uint64_t ingest_mode,
                             RequestCallback callback) = 0;

    //! Stop processing threads
    //! used e.g. for Python bindings
    virtual void StopThreads__() = 0;

    //! Sends files to the default substream
    /*!
      \param event_header - A stucture with the meta information (file name, size is ignored).
      \param full_path - A full path of the file to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendFile(const EventHeader& event_header, std::string full_path, uint64_t ingest_mode,
                           RequestCallback callback) = 0;

    //! Sends files to the substream
    /*!
      \param event_header - A stucture with the meta information (file name, size is ignored).
      \param full_path - A full path of the file to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendFile(const EventHeader& event_header, std::string substream, std::string full_path,
                           uint64_t ingest_mode,
                           RequestCallback callback) = 0;

    //! Marks substream finished
    /*!
      \param substream - Name of the substream to makr finished
      \param last_id - ID of the last image in substream
      \param next_substream - Name of the next substream (empty if not set)
      \return Error - Will be nullptr on success
    */
    virtual Error SendSubstreamFinishedFlag(std::string substream, uint64_t last_id, std::string next_substream,
                                            RequestCallback callback) = 0;


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
    virtual Error SetCredentials(SourceCredentials source_cred) = 0;
    //! Set get current size of the requests queue
    virtual  uint64_t  GetRequestsQueueSize() = 0;
    //! Wait until all current requests are processed or timeout
    virtual Error WaitRequestsFinished(uint64_t timeout_ms) = 0;

};
}

#endif //ASAPO_PRODUCER__PRODUCER_H
