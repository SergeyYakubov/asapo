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

class Consumer {
  public:
    //! Reset counter for the specific group.
    /*!
      \param group_id - group id to use.
      \return nullptr of command was successful, otherwise error.
    */
    virtual Error ResetLastReadMarker(std::string group_id) = 0;
    virtual Error ResetLastReadMarker(std::string group_id, std::string stream) = 0;

    virtual Error SetLastReadMarker(uint64_t value, std::string group_id) = 0;
    virtual Error SetLastReadMarker(uint64_t value, std::string group_id, std::string stream) = 0;

    //! Acknowledge data tuple for specific group id and stream.
    /*!
        \param group_id - group id to use.
        \param id - data tuple id
        \param stream (optional) - stream
        \return nullptr of command was successful, otherwise error.
    */
    virtual Error Acknowledge(std::string group_id, uint64_t id, std::string stream = kDefaultStream) = 0;

    //! Negative acknowledge data tuple for specific group id and stream.
    /*!
        \param group_id - group id to use.
        \param id - data tuple id
        \param delay_ms - data tuple will be redelivered after delay, 0 to redeliver immediately
        \param stream (optional) - stream
        \return nullptr of command was successful, otherwise error.
    */
    virtual Error NegativeAcknowledge(std::string group_id, uint64_t id, uint64_t delay_ms,
                                      std::string stream = kDefaultStream) = 0;


    //! Get unacknowledged tuple for specific group id and stream.
    /*!
        \param group_id - group id to use.
        \param stream (optional) - stream
        \param from_id - return tuples with ids greater or equal to from (use 0 disable limit)
        \param to_id - return tuples with ids less or equal to to (use 0 to disable limit)
        \param in (optional) - stream
        \param err - set to nullptr of operation succeed, error otherwise.
        \return vector of ids, might be empty
    */
    virtual IdList GetUnacknowledgedTupleIds(std::string group_id, std::string stream, uint64_t from_id, uint64_t to_id,
                                             Error* error) = 0;
    virtual IdList GetUnacknowledgedTupleIds(std::string group_id, uint64_t from_id, uint64_t to_id, Error* error) = 0;

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

    //! Get list of streams, set from to "" to get all streams
    virtual StreamInfos GetStreamList(std::string from, Error* err) = 0;

    //! Get current number of datasets
    /*!
      \param err - return nullptr of operation succeed, error otherwise.
      \return number of datasets.
    */
    virtual uint64_t GetCurrentSize(Error* err) = 0;
    virtual uint64_t GetCurrentSize(std::string stream, Error* err) = 0;

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
      \param group_id - group id to use.
      \param data - where to store message data. Can be set to nullptr only message metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetNext(MessageMeta* info, std::string group_id, MessageData* data) = 0;
    virtual Error GetNext(MessageMeta* info, std::string group_id, std::string stream, MessageData* data) = 0;

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
      \param stream - stream to use ("" for default).
      \param min_size - wait until dataset has min_size data tuples (0 for maximum size)
      \return DataSet - information about the dataset

    */
    virtual DataSet GetNextDataset(std::string group_id, std::string stream, uint64_t min_size, Error* err) = 0;
    virtual DataSet GetNextDataset(std::string group_id, uint64_t min_size, Error* err) = 0;
    //! Receive last available dataset which has min_size data tuples.
    /*!
      \param err -  will be set to error data cannot be read, nullptr otherwise.
      \param stream - stream to use ("" for default).
      \param min_size - amount of data tuples in dataset (0 for maximum size)
      \return DataSet - information about the dataset
    */
    virtual DataSet GetLastDataset(std::string stream, uint64_t min_size, Error* err) = 0;
    virtual DataSet GetLastDataset(uint64_t min_size, Error* err) = 0;

    //! Receive dataset by id.
    /*!
      \param id - dataset id
      \param err -  will be set to error data cannot be read or dataset size less than min_size, nullptr otherwise.
      \param stream - stream to use ("" for default).
      \param min_size - wait until dataset has min_size data tuples (0 for maximum size)
      \return DataSet - information about the dataset
    */
    virtual DataSet GetDatasetById(uint64_t id, std::string stream, uint64_t min_size, Error* err) = 0;
    virtual DataSet GetDatasetById(uint64_t id, uint64_t min_size, Error* err) = 0;

    //! Receive single message by id.
    /*!
      \param id - message id
      \param info -  where to store message metadata. Can be set to nullptr only message data is needed.
      \param data - where to store message data. Can be set to nullptr only message metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetById(uint64_t id, MessageMeta* info, MessageData* data) = 0;
    virtual Error GetById(uint64_t id, MessageMeta* info, std::string stream, MessageData* data) = 0;

    //! Receive id of last acknowledged data tuple
    /*!
      \param group_id - group id to use.
      \param stream (optional) - stream
      \param err -  will be set in case of error, nullptr otherwise.
      \return id of the last acknowledged message, 0 if error
    */
    virtual uint64_t GetLastAcknowledgedTulpeId(std::string group_id, std::string stream, Error* error) = 0;
    virtual uint64_t GetLastAcknowledgedTulpeId(std::string group_id, Error* error) = 0;

    //! Receive last available message.
    /*!
      \param info -  where to store message metadata. Can be set to nullptr only message data is needed.
      \param data - where to store message data. Can be set to nullptr only message metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetLast(MessageMeta* info, MessageData* data) = 0;
    virtual Error GetLast(MessageMeta* info, std::string stream, MessageData* data) = 0;

    //! Get all messages matching the query.
    /*!
      \param sql_query -  query string in SQL format. Limit subset is supported
      \param err - will be set in case of error, nullptr otherwise
      \return vector of message metadata matchiing to specified query. Empty if nothing found or error
    */
    virtual MessageMetas QueryMessages(std::string query, Error* err) = 0;
    virtual MessageMetas QueryMessages(std::string query, std::string stream, Error* err) = 0;

    //! Configure resending nonacknowledged data
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
