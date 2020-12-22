#ifndef ASAPO_PRODUCER__PRODUCER_H
#define ASAPO_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

#include "asapo/logger/logger.h"
#include "common.h"
#include "asapo/common/data_structs.h"

namespace asapo {

class Producer {
  public:
    //! Creates a new producer
    /*!
     * @return A unique_ptr to a new producer instance
     */
    static std::unique_ptr<Producer> Create(const std::string& endpoint, uint8_t n_processing_threads,
                                            asapo::RequestHandlerType type, SourceCredentials source_cred,
                                            uint64_t timeout_ms,
                                            Error* err);

    virtual ~Producer() = default;

    //! Get stream information from receiver
    /*!
      \param stream (optional) - stream
      \param timeout_ms - operation timeout in milliseconds
      \return StreamInfo - a structure with stream information
    */
    virtual StreamInfo GetStreamInfo(std::string stream, uint64_t timeout_ms, Error* err) const = 0;
    virtual StreamInfo GetStreamInfo(uint64_t timeout_ms, Error* err) const = 0;

  //! Get stream that has the newest ingested data
  /*!
    \param timeout_ms - operation timeout in milliseconds
    \return StreamInfo - a structure with stream information
  */
    virtual StreamInfo GetLastStream(uint64_t timeout_ms, Error* err) const = 0;


    //! Sends message to the receiver
    /*!
      \param message_header - A stucture with the meta information (file name, size, a string with user metadata (JSON format)).
      \param data - A smart pointer to the message data to send
      \return Error - Will be nullptr on success
    */
    virtual Error Send(const MessageHeader& message_header, MessageData data, uint64_t ingest_mode,
                       RequestCallback callback) = 0;


    //! Sends message to the receiver - same as Send - memory should not be freed until send is finished
    //! used e.g. for Python bindings
    virtual Error Send__(const MessageHeader& message_header, void* data, uint64_t ingest_mode,
                         RequestCallback callback) = 0;

    //! Sends message to the receiver
    /*!
      \param message_header - A stucture with the meta information (file name, size, a string with user metadata (JSON format)).
      \param data - A smart pointer to the message data to send, can be nullptr
      \return Error - Will be nullptr on success
    */
    virtual Error Send(const MessageHeader& message_header, std::string stream, MessageData data, uint64_t ingest_mode,
                       RequestCallback callback) = 0;


    //! Sends data to the receiver - same as Send - memory should not be freed until send is finished
    //! used e.g. for Python bindings
    virtual Error Send__(const MessageHeader& message_header, std::string stream, void* data, uint64_t ingest_mode,
                         RequestCallback callback) = 0;

    //! Stop processing threads
    //! used e.g. for Python bindings
    virtual void StopThreads__() = 0;

    //! Sends message from a file to the default stream
    /*!
      \param message_header - A stucture with the meta information (file name, size is ignored).
      \param full_path - A full path of the file to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendFromFile(const MessageHeader& message_header, std::string full_path, uint64_t ingest_mode,
                               RequestCallback callback) = 0;

    //! Sends message from a file to a stream
    /*!
      \param message_header - A stucture with the meta information (file name, size is ignored).
      \param full_path - A full path of the file to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendFromFile(const MessageHeader& message_header, std::string stream, std::string full_path,
                               uint64_t ingest_mode,
                               RequestCallback callback) = 0;

    //! Marks stream finished
    /*!
      \param stream - Name of the stream to makr finished
      \param last_id - ID of the last image in stream
      \param next_stream - Name of the next stream (empty if not set)
      \return Error - Will be nullptr on success
    */
    virtual Error SendStreamFinishedFlag(std::string stream, uint64_t last_id, std::string next_stream,
                                            RequestCallback callback) = 0;


    //! Sends metadata for the current beamtime to the receiver
    /*!
      \param metadata - a JSON string with metadata
      \param callback - callback function
      \return Error - will be nullptr on success
    */
    virtual Error SendMetadata(const std::string& metadata, RequestCallback callback) = 0;

    //! Set internal log level
    virtual void SetLogLevel(LogLevel level) = 0;
    //! Enables/Disables logs output to stdout
    virtual void EnableLocalLog(bool enable) = 0;
    //! Enables/Disables sending logs to the central server
    virtual void EnableRemoteLog(bool enable) = 0;
    //! Set beamtime id which producer will use to send data
    virtual Error SetCredentials(SourceCredentials source_cred) = 0;
    //! Get current size of the requests queue (number of requests pending/being processed)
    virtual  uint64_t  GetRequestsQueueSize() = 0;
    //! Get current volume of the requests queue (total memory of occupied by pending/being processed requests)
    virtual  uint64_t  GetRequestsQueueVolumeMb() = 0;
    //! Set maximum size of the requests queue (0 for unlimited, default 0) and volume in Megabytes (0 for unlimited, default 0)
    virtual  void SetRequestsQueueLimits(uint64_t size, uint64_t volume) = 0;
    //! Wait until all current requests are processed or timeout
    virtual Error WaitRequestsFinished(uint64_t timeout_ms) = 0;

};
}

#endif //ASAPO_PRODUCER__PRODUCER_H
