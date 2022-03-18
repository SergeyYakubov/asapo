#ifndef ASAPO_PRODUCER__PRODUCER_H
#define ASAPO_PRODUCER__PRODUCER_H

#include <memory>
#include <string>

#include "asapo/logger/logger.h"
#include "common.h"
#include "asapo/common/data_structs.h"
#include "asapo/preprocessor/deprecated.h"

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

    //! Return version
    /*!
      \param client_info - for client version
      \param server_info - for server
      \param supported - set to true if client is supported by server
      \return nullptr of command was successful, otherwise error.
    */
    virtual Error GetVersionInfo(std::string* client_info, std::string* server_info, bool* supported) const = 0;

    //! Get stream information from receiver
    /*!
      \param stream - stream to send messages to
      \param timeout_ms - operation timeout in milliseconds
      \return StreamInfo - a structure with stream information
    */
    virtual StreamInfo GetStreamInfo(std::string stream, uint64_t timeout_ms, Error* err) const = 0;

    //! Get stream metadata from receiver
    /*!
      \param stream - stream to send messages to
      \param timeout_ms - operation timeout in milliseconds
      \return JSON string with metadata
    */
    virtual std::string GetStreamMeta(const std::string& stream, uint64_t timeout_ms, Error* err) const = 0;

    //! Get beamtime metadata from receiver
    /*!
      \param timeout_ms - operation timeout in milliseconds
      \return JSON string with metadata
    */
    virtual std::string GetBeamtimeMeta(uint64_t timeout_ms, Error* err) const = 0;

    //! Delete stream
    /*!
      \param stream - stream to send messages to
      \param timeout_ms - operation timeout in milliseconds
      \param options - delete stream options
      \return Error - will be nullptr on success
    */
    virtual Error DeleteStream(std::string stream, uint64_t timeout_ms, DeleteStreamOptions options) const = 0;

    //! Get stream that has the newest ingested data
    /*!
      \param timeout_ms - operation timeout in milliseconds
      \return StreamInfo - a structure with stream information
    */
    virtual StreamInfo GetLastStream(uint64_t timeout_ms, Error* err) const = 0;

    //! Sends message to the receiver
    /*!
      \param message_header - A stucture with the meta information (file name, size, a string with user metadata (JSON format)).
      \param data - A smart pointer to the message data to send, can be nullptr
      \return Error - Will be nullptr on success
    */
    virtual Error Send(const MessageHeader& message_header,
                       MessageData data,
                       uint64_t ingest_mode,
                       std::string stream,
                       RequestCallback callback) = 0;


    //! Sends data to the receiver - same as Send - memory should not be freed after send is finished
    //! used e.g. for Python bindings
    virtual Error Send__(const MessageHeader& message_header,
                         void* data,
                         uint64_t ingest_mode,
                         std::string stream,
                         RequestCallback callback) = 0;

    //! Stop processing threads
    //! used e.g. for Python bindings
    virtual void StopThreads__() = 0;

    //! Sends message from a file to a stream
    /*!
      \param message_header - A stucture with the meta information (file name, size is ignored).
      \param file_to_send - A full path of the file to send
      \return Error - Will be nullptr on success
    */
    virtual Error SendFile(const MessageHeader& message_header,
                           std::string file_to_send,
                           uint64_t ingest_mode,
                           std::string stream,
                           RequestCallback callback) = 0;

    //! Marks stream finished
    /*!
      \param stream - Name of the stream to makr finished
      \param last_id - ID of the last message in stream
      \param next_stream - Name of the next stream (empty if not set)
      \return Error - Will be nullptr on success
    */
    virtual Error SendStreamFinishedFlag(std::string stream, uint64_t last_id, std::string next_stream,
                                         RequestCallback callback) = 0 ;


    //! Sends beamtime metadata to the receiver
    /*!
      \deprecated { deprecated, obsolates 01.07.2022, use SendBeamtimeMetadata instead}
      \param metadata - a JSON string with metadata
      \param callback - callback function
      \return Error - will be nullptr on success
    */
    virtual Error ASAPO_DEPRECATED("obsolates 01.07.2022, use SendBeamtimeMetadata instead") SendMetadata(
        const std::string& metadata,
        RequestCallback callback)  = 0;

    //! Sends beamtime metadata to the receiver
    /*!
      \param metadata - a JSON string with metadata
      \param MetaIngestMode - a JSON string with metadata
      \param callback - callback function
      \return Error - will be nullptr on success
    */
    virtual Error SendBeamtimeMetadata(const std::string& metadata, MetaIngestMode mode, RequestCallback callback) = 0;

    //! Sends stream metadata to the receiver
    /*!
      \param stream - name of the stream
      \param metadata - a JSON string with metadata
      \param callback - callback function
      \return Error - will be nullptr on success
    */
    virtual Error SendStreamMetadata(const std::string& metadata,
                                     MetaIngestMode mode,
                                     const std::string& stream,
                                     RequestCallback callback) = 0;

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
