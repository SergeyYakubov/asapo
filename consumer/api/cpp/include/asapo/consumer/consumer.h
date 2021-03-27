#ifndef ASAPO_DATASOURCE_H
#define ASAPO_DATASOURCE_H

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "asapo/common/data_structs.h"
#include "asapo/common/error.h"
#include "asapo/common/networking.h"

namespace asapo {

enum class StreamFilter {
  kAllStreams,
  kFinishedStreams,
  kUnfinishedStreams
};

class Consumer {
  public:
    //! Reset counter for the specific group.
    /*!
      \param group_id - group id to use.
      \param stream - stream to use
      \return nullptr of command was successful, otherwise error.
    */
    virtual Error ResetLastReadMarker(std::string group_id, std::string stream) = 0;
  //! Return version
  /*!
    \param client_info - for client version
    \param server_info - for server
    \param supported - set to true if client is supported by server
    \return nullptr of command was successful, otherwise error.
  */
    virtual Error GetVersionInfo(std::string* client_info,std::string* server_info, bool* supported) = 0;

    virtual Error SetLastReadMarker(std::string group_id, uint64_t value, std::string stream) = 0;

    //! Acknowledge message for specific group id and stream.
    /*!
        \param group_id - group id to use.
        \param id - message id
        \param stream - stream to use
        \return nullptr of command was successful, otherwise error.
    */
    virtual Error Acknowledge(std::string group_id, uint64_t id, std::string stream) = 0;

    //! Negative acknowledge message for specific group id and stream.
    /*!
        \param group_id - group id to use.
        \param id - message id
        \param delay_ms - message will be redelivered after delay, 0 to redeliver immediately
        \param stream - stream to use
        \return nullptr of command was successful, otherwise error.
    */
    virtual Error NegativeAcknowledge(std::string group_id, uint64_t id, uint64_t delay_ms,
                                      std::string stream) = 0;


    //! Get unacknowledged messages for specific group id and stream.
    /*!
        \param group_id - group id to use.
        \param from_id - return messages with ids greater or equal to from (use 0 disable limit)
        \param to_id - return messages with ids less or equal to to (use 0 to disable limit)
        \param stream - stream to use
        \return nullptr if operation succeed, error otherwise.
    */
    virtual IdList GetUnacknowledgedMessages(std::string group_id,
                                             uint64_t from_id,
                                             uint64_t to_id,
                                             std::string stream,
                                             Error* error) = 0;

    //! Set timeout for consumer operations. Default - no timeout
    virtual void SetTimeout(uint64_t timeout_ms) = 0;

    //! Will disable RDMA.
    //! If RDMA is disabled, not available or the first connection fails to build up, it will automatically fall back to TCP.
    //! This will only have an effect if no previous connection attempted was made on this Consumer.
    virtual void ForceNoRdma() = 0;

    //! Returns the current network connection type
    /*!
     * \return current network connection type. If no connection was made, the result is NetworkConnectionType::kUndefined
     */
    virtual NetworkConnectionType CurrentConnectionType() const = 0;

  //! Get list of streams with filter, set from to "" to get all streams
    virtual StreamInfos GetStreamList(std::string from,  StreamFilter filter, Error* err) = 0;

    //! Get current number of messages in stream
    /*!
      \param stream - stream to use
      \param err - return nullptr of operation succeed, error otherwise.
      \return number of datasets.
    */
    virtual uint64_t GetCurrentSize(std::string stream, Error* err) = 0;

  //! Get current number of datasets in stream
  /*!
    \param stream - stream to use
    \param include_incomplete - flag to count incomplete datasets as well
    \param err - return nullptr of operation succeed, error otherwise.
    \return number of datasets.
  */
    virtual uint64_t GetCurrentDatasetCount(std::string stream, bool include_incomplete, Error* err) = 0;

  //! Generate new GroupID.
  /*!
    \param err - return nullptr of operation succeed, error otherwise.
    \return group ID.
  */

    virtual std::string GenerateNewGroupId(Error* err) = 0;

    //! Get Beamtime metadata.
    /*!
      \param err - return nullptr of operation succeed, error otherwise.
      \return beamtime metadata.
    */
    virtual std::string GetBeamtimeMeta(Error* err) = 0;

    //! Receive next available message.
    /*!
      \param info -  where to store message metadata. Can be set to nullptr only message data is needed.
      \param group_id - group id to use
      \param data - where to store message data. Can be set to nullptr only message metadata is needed.
      \param stream - stream to use
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetNext(std::string group_id, MessageMeta* info, MessageData* data, std::string stream) = 0;

    //! Retrieves message using message meta.
    /*!
      \param info - message metadata to use, can be updated after operation
      \param data - where to store message data. Can be set to nullptr only message metadata is needed.
      \return Error if data is nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error RetrieveData(MessageMeta* info, MessageData* data) = 0;


    //! Receive next available completed dataset.
    /*!
      \param err -  will be set to error data cannot be read, nullptr otherwise.
      \param group_id - group id to use.
      \param min_size - wait until dataset has min_size messages (0 for maximum size)
      \param stream - stream to use
      \return DataSet - information about the dataset

    */
    virtual DataSet GetNextDataset(std::string group_id, uint64_t min_size, std::string stream, Error* err) = 0;
    //! Receive last available dataset which has min_size messages.
    /*!
      \param err -  will be set to error data cannot be read, nullptr otherwise.
      \param min_size - amount of messages in dataset (0 for maximum size)
      \param stream - stream to use
      \return DataSet - information about the dataset
    */
    virtual DataSet GetLastDataset(uint64_t min_size, std::string stream, Error* err) = 0;

    //! Receive dataset by id.
    /*!
      \param id - dataset id
      \param err -  will be set to error data cannot be read or dataset size less than min_size, nullptr otherwise.
      \param min_size - wait until dataset has min_size messages (0 for maximum size)
      \param stream - stream to use
      \return DataSet - information about the dataset
    */
    virtual DataSet GetDatasetById(uint64_t id, uint64_t min_size, std::string stream, Error* err) = 0;

    //! Receive single message by id.
    /*!
      \param id - message id
      \param info -  where to store message metadata. Can be set to nullptr only message data is needed.
      \param data - where to store message data. Can be set to nullptr only message metadata is needed.
      \param stream - stream to use
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetById(uint64_t id, MessageMeta* info, MessageData* data, std::string stream) = 0;

    //! Receive id of last acknowledged message
    /*!
      \param group_id - group id to use.
      \param stream - stream to use
      \param error -  will be set in case of error, nullptr otherwise.
      \return id of the last acknowledged message, 0 if error
    */
    virtual uint64_t GetLastAcknowledgedMessage(std::string group_id, std::string stream, Error* error) = 0;

    //! Receive last available message.
    /*!
      \param info -  where to store message metadata. Can be set to nullptr only message data is needed.
      \param data - where to store message data. Can be set to nullptr only message metadata is needed.
      \param stream - stream to use
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetLast(MessageMeta* info, MessageData* data, std::string stream) = 0;

    //! Get all messages matching the query.
    /*!
      \param sql_query -  query string in SQL format. Limit dataset is supported
      \param stream - stream to use
      \param err - will be set in case of error, nullptr otherwise
      \return vector of message metadata matchiing to specified query. Empty if nothing found or error
    */
    virtual MessageMetas QueryMessages(std::string query, std::string stream, Error* err) = 0;

    //! Configure resending unacknowledged data
    /*!
      \param resend -  where to resend
      \param delay_ms - how many milliseconds to wait before resending
      \param resend_attempts - how many resend attempts to make
    */
    virtual void SetResendNacs(bool resend, uint64_t delay_ms, uint64_t resend_attempts) = 0;

  //! Will try to interrupt current long runnung operations (mainly needed to exit waiting loop in C from Python)
    virtual void InterruptCurrentOperation() = 0;

    virtual ~Consumer() = default; // needed for unique_ptr to delete itself
};

/*! A class to create consumer instance. The class's only function Create is used for this */
class ConsumerFactory {
  public:
    static std::unique_ptr<Consumer> CreateConsumer(std::string server_name, std::string source_path,
                                                    bool has_filesystem, SourceCredentials source, Error* error) noexcept;

};

}
#endif //ASAPO_DATASOURCE_H
