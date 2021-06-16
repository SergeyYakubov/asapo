#define __CONSUMER_C_INTERFACE_IMPLEMENTATION__
#include "asapo/asapo_consumer.h"
//! boolean type
typedef bool asapoBool;


//! handle for an asapo consumer
/// created by asapoCreateConsumer()
/// delete after use with asapoDeleteConsumer()
/// all operations are done vis tha asapoConsumerXxx() functions
/// \sa asapo::Consumer
typedef asapo::Consumer* asapoConsumer;

//! handle for credentials to acess a source from a consumer
/// created by asapoCreateSourceCredentials()
/// delete after deletion of consumer with asapoDeleteSourceCredentials()
/// \sa asapo::SourceCredentials
typedef asapo::SourceCredentials* asapoSourceCredentials;

//! handle for an asapo error
/// either a return value, NULL if no error
/// or an output parameter, then a pointer to an asapoError will be used and set to NULL or something
/// needs to be cleared after use with asapoClearError()
/// text version of an error: asapoErrorExplain()
/// enum value of the error: asapoErrorGetType(), \sa ::asapoErrorType asapo::ErrorType
typedef asapo::ErrorInterface* asapoError;

//! handle for metadata of a message
/// create with asapoCreateMessageMeta()
/// delete after use with asapoDeleteMessageMeta()
/// A set of getters asapoMessageMetaGetXxx() are defined
/// \sa asapo::MessageMeta
typedef asapo::MessageMeta* asapoMessageMeta;


//! handle for data recieved by the consumer
/// set as outout parameter via asapoConsumerGetNext(), asapoConsumerGetLast()
/// delete after use with asapoDeleteMessageData()
/// access to the data is granted via  asapoMessageDataGetAsChars()
typedef uint8_t* asapoMessageData;

//! handle for a consumer group id
/// create with asapoConsumerGenerateNewGroupId()
/// delete after use with asapoDeleteGroupId()
typedef std::string* asapoGroupId;

//! handle for info about a stream,
/// object is deleted implicityly by  asapoDeleteStreamInfos()
/// may be set via asapoStreamInfosGetInfo()
/// \sa asapo::StreamInfo asapoStreamInfoGetLast_id() asapoStreamInfoGetName() asapoStreamInfoGetFfinished()  asapoStreamInfoGetNext_stream()  asapoStreamInfoGetTimestampCreated() asapoStreamInfoGetTimestamoLastEntry()
typedef asapo::StreamInfo* asapoStreamInfo;

//! handle for a set of stream infos
/// touch only with proper functions and use asapoDeleteStreamInfos() to delete,
/// created by asapoConsumerGetStreamList()
/// \sa asapoDeleteStreamInfos() asapoStreamInfosGetItem() asapoStreamInfosGetSize()
typedef asapo::StreamInfos* asapoStreamInfos;


//! handle for massage id lists
/// touch only with proper functions and use asapoDeleteIdList() to delete,
/// created by asapoConsumerGetUnacknowledgedMessages()
/// \sa asapo::IdList asapoIdListGetSize() asapoIdListGetItem()
typedef asapo::IdList* asapoIdList;






#include <algorithm>

template <typename t> constexpr bool operator==(unsigned lhs, t rhs) {
    return lhs == static_cast<typename std::underlying_type<t>::type>(rhs);
}

extern "C" {
#include "asapo/consumer_c.h"
    static_assert(kUnknownError == asapo::ErrorType::kUnknownError&&
                  kAsapoError == asapo::ErrorType::kAsapoError&&
                  kHttpError == asapo::ErrorType::kHttpError&&
                  kIOError == asapo::ErrorType::kIOError&&
                  kDBError == asapo::ErrorType::kDBError&&
                  kReceiverError == asapo::ErrorType::kReceiverError&&
                  kProducerError == asapo::ErrorType::kProducerError&&
                  kConsumerError == asapo::ErrorType::kConsumerError&&
                  kMemoryAllocationError == asapo::ErrorType::kMemoryAllocationError&&
                  kEndOfFile == asapo::ErrorType::kEndOfFile&&
                  kFabricError == asapo::ErrorType::kFabricError,
                  "incompatible bit reps between c++ and c for asapo::ErrorType");
    static_assert(kAllStreams == asapo::StreamFilter::kAllStreams&&
                  kFinishedStreams == asapo::StreamFilter::kFinishedStreams&&
                  kUnfinishedStreams == asapo::StreamFilter::kUnfinishedStreams,
                  "incompatible bit reps between c++ and c for asapo::StreamFilter");
    static_assert(kProcessed == asapo::SourceType::kProcessed&&
                  kRaw == asapo::SourceType::kRaw,
                  "incompatible bit reps between c++ and c for asapo::SourceType");
    static_assert(kUndefined == asapo::NetworkConnectionType::kUndefined&&
                  kAsapoTcp == asapo::NetworkConnectionType::kAsapoTcp&&
                  kFabric == asapo::NetworkConnectionType::kFabric,
                  "incompatible bit reps between c++ and c for asapo::NetworkConnectionType");

    static void timePointToTimeSpec(std::chrono::system_clock::time_point tp,
                                    struct timespec* stamp) {
        stamp->tv_sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
        stamp->tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count() % 1000000000;
    }

    /// \copydoc asapo::ErrorInterface::Explain()
    /// \param[out] buf will be filled with the explanation
    /// \param[in] maxSize max size of buf in bytes
    void asapoErrorExplain(const asapoError error, char* buf, size_t maxSize) {
        if (error) {
            auto explanation = error->Explain().substr(0, maxSize - 1);
            std::copy(explanation.begin(), explanation.end(), buf);
            buf[explanation.size()] = '\0';
        } else {
            static std::string msg("no error");
            std::copy_n(msg.begin(), std::max(msg.size(), maxSize), buf);
            buf[std::max(maxSize - 1, msg.size())] = '\0';
        }
    }
    /// \copydoc asapo::ErrorInterface::GetErrorType()
    enum asapoErrorType asapoErrorGetType(const asapoError error) {
        return static_cast<asapoErrorType>(error->GetErrorType());
    }
    //! clean up error
    /// frees the resources occupied by error,
    /// sets *error to NULL
    void asapoClearError(asapoError* error) {
        delete *error;
        error = nullptr;
    }


    //! creata a consumer
    /// \copydoc asapo::ConsumerFactory::CreateConsumer
    /// return handle to the created cosumer
    asapoConsumer asapoCreateConsumer(const char* server_name,
                                      const char* source_path,
                                      asapoBool has_filesysytem,
                                      asapoSourceCredentials source,
                                      asapoError* error) {


        asapo::Error err;
        auto c = asapo::ConsumerFactory::CreateConsumer(server_name,
                                                        source_path,
                                                        has_filesysytem,
                                                        *source,
                                                        &err);
        *error = err.release();
        return c.release();
    }

    //! clean up consumer
    /// frees the resources occupied by consumer, sets *consumer to NULL
    /// \param[in] consumer the handle of the consumer concerned
    void asapoDeleteConsumer(asapoConsumer* consumer) {
        delete *consumer;
        *consumer = nullptr;
    }
    //! wraps asapo::Consumer::GenerateNewGroupId()
    /// \copydoc asapo::Consumer::GenerateNewGroupId()
    /// \param[in] consumer the handle of the consumer concerned
    asapoGroupId asapoConsumerGenerateNewGroupId(asapoConsumer consumer,
                                                 asapoError* error) {
        asapo::Error err;
        auto result = new std::string(consumer->GenerateNewGroupId(&err));
        *error = err.release();
        return result;
    }

    //! clean up groupId
    /// frees the resources occupied by id, sets *id to NULL
    void asapoDeleteGroupId(asapoGroupId* id) {
        delete *id;
        *id = nullptr;
    }

    //! wraps asapo::Consumer::SetTimeout()
    /// \copydoc asapo::Consumer::SetTimeout()
    /// \param[in] consumer the handle of the consumer concerned
    void asapoConsumerSetTimeout(asapoConsumer consumer, uint64_t timeout_ms) {
        consumer->SetTimeout(timeout_ms);
    }

    //! wraps asapo::Consumer::ResetLastReadMarker()
    /// \copydoc asapo::Consumer::ResetLastReadMarker()
    /// \param[in] consumer the handle of the consumer concerned
    asapoError asapoConsumerResetLastReadMarker(asapoConsumer consumer,
                                                const asapoGroupId group_id,
                                                const char* stream) {
        auto err = consumer->ResetLastReadMarker(*group_id, stream);
        return err.release();
    }

    //! wraps asapo::Consumer::SetLastReadMarker()
    /// \copydoc asapo::Consumer::SetLastReadMarker()
    /// \param[in] consumer the handle of the consumer concerned
    asapoError asapoConsumerSetLastReadMarker(asapoConsumer consumer,
                                              const asapoGroupId group_id,
                                              uint64_t value,
                                              const char* stream) {
        auto err = consumer->SetLastReadMarker(*group_id, value, stream);
        return err.release();
    }
    //! wraps asapo::Consumer::Acknowledge()
    /// \copydoc asapo::Consumer::Acknowledge()
    /// \param[in] consumer the handle of the consumer concerned
    asapoError asapoConsumerAcknowledge(asapoConsumer consumer,
                                        const asapoGroupId group_id,
                                        uint64_t id,
                                        const char* stream) {
        auto err = consumer->Acknowledge(*group_id, id, stream);
        return err.release();
    }
    //! wraps asapo::Consumer::NegativeAcknowledge()
    /// \copydoc asapo::Consumer::NegativeAcknowledge()
    /// \param[in] consumer the handle of the consumer concerned
    asapoError asapoConsumerNegativeAcknowledge(asapoConsumer consumer,
                                                const asapoGroupId group_id,
                                                uint64_t id,
                                                uint64_t delay_ms,
                                                const char* stream) {
        auto err = consumer->NegativeAcknowledge(*group_id, id, delay_ms, stream);
        return err.release();
    }

    //! wraps asapo::Consumer::GetUnacknowledgedMessages()
    /// \copydoc asapo::Consumer::GetUnacknowledgedMessages()
    /// \param[in] consumer the handle of the consumer concerned
    asapoIdList asapoConsumerGetUnacknowledgedMessages(asapoConsumer consumer,
            asapoGroupId group_id,
            uint64_t from_id,
            uint64_t to_id,
            const char* stream,
            asapoError* error) {
        asapo::Error err;
        auto list = new asapo::IdList(consumer->GetUnacknowledgedMessages(*group_id,
                                      from_id, to_id,
                                      stream,
                                      &err));
        *error = err.release();
        return list;
    }
    //! cleans up an IdList
    void asapoDeleteIdList(asapoIdList* list) {
        delete *list;
        *list = nullptr;
    }
    //! give number of items in an id list
    /// \param[in] list handle of an id list
    size_t asapoIdListGetSize(const asapoIdList list) {
        return list->size();
    }
    //! give one items from an id list
    /// \param[in] list handle of an id list
    /// \param[in] index index of the item to return, start at 0
    uint64_t asapoIdListGetItem(const asapoIdList list,
                                size_t index) {
        return list->at(index);
    }
    //! wraps asapo::Consumer::ForceNoRdma()
    /// \copydoc asapo::Consumer::ForceNoRdma()
    /// \param[in] consumer the handle of the consumer concerned
    void asapoConsumerForceNoRdma(asapoConsumer consumer);
    //! wraps asapo::Consumer::CurrentConnectionType()
    /// \copydoc asapo::Consumer::CurrentConnectionType()
    /// \param[in] consumer the handle of the consumer concerned
    enum asapoNetworkConnectionType asapoConsumerCurrentConnectionType(asapoConsumer consumer);


    //! get list of streams, wraps asapo::Consumer::GetStreamList()
    /*!
      \param[in] consumer the consumer that is acted upon
      \param[in] from whatever
      \param[in] filter select the kind of stream
      \param[out] error will contain a pointer to an asapoError if a problem occured, NULL else.
      \return object that contains the stream infos
      \sa asapoStreamInfosGetInfo() asapoStreamInfosGetSize() asapoDeleteStreamInfos()
    */
    asapoStreamInfos asapoConsumerGetStreamList(asapoConsumer consumer,
                                                const char* from,
                                                enum asapoStreamFilter filter,
                                                asapoError* error) {
        asapo::Error err;
        auto info = new asapo::StreamInfos(consumer->GetStreamList(from,
                                           static_cast<asapo::StreamFilter>(filter),
                                           &err));
        *error = err.release();
        return info;
    }

    //! get one stream info from a stream infos handle
    /// \param[in] infos handle for stream infos
    /// \param[in] index index od info to get, starts at 0
    /// \return handle to stream info, do not delete!
    const asapoStreamInfo asapoStreamInfosGetItem(const asapoStreamInfos infos,
                                                  size_t index) {
        return &infos->at(index);
    }
    //! get size (number of elements) of a stream infos handle
    /// \param[in] infos handle for stream infos
    /// \return number of elements in the handle
    size_t asapoStreamInfosGetSize(const asapoStreamInfos infos) {
        return infos->size();
    }
    //! clean up streamInfos
    /// frees the resources occupied by infos, sets *infos to NULL

    void asapoDeleteStreamInfos(asapoStreamInfos* infos) {
        delete *infos;
        *infos = nullptr;
    }

    //! wraps asapo::Consumer::DeleteStream()
    /// \copydoc asapo::Consumer::DeleteStream()
    /// \param[in] consumer the consumer that is acted upon
    /// \param[in] delete_meta the delete_meta part of the asapo::DeleteStreamOptions
    /// \param[in] error_on_not_exist the error_on_not_exist part of the asapo::DeleteStreamOptions
    asapoError asapoConsumerDeleteStream(asapoConsumer consumer,
                                         const char* stream,
                                         asapoBool delete_meta,
                                         asapoBool error_on_not_exist) {
        asapo::DeleteStreamOptions opt(delete_meta, error_on_not_exist);
        auto err = consumer->DeleteStream(stream, opt);
        return err.release();
    }
    //! wraps asapo::Consumer::GetCurrentSize()
    /// \copydoc asapo::Consumer::GetCurrentSize()
    /// \param[in] consumer the consumer that is acted upon
    uint64_t asapoConsumerGetCurrentSize(asapoConsumer consumer,
                                         const char* stream,
                                         asapoError* error) {
        asapo::Error err;
        auto retval = consumer->GetCurrentSize(stream, &err);
        *error = err.release();
        return retval;
    }
    //! wraps asapo::Consumer::GetCurrentDatasetCount()
    /// \copydoc asapo::Copydoc::GetCurrentDatasetCount()
    /// \param[in] consumer the consumer that is acted upon
    uint64_t asapoConsumerGetCurrentDatasetCount(asapoConsumer consumer,
                                                 const char* stream,
                                                 asapoBool include_incomplete,
                                                 asapoError* error) {
        asapo::Error err;
        auto retval = consumer->GetCurrentDatasetCount(stream, include_incomplete, &err);
        *error = err.release();
        return retval;
    }




    //! wraps asapo::Consumer::GetLast()
    /// \copydoc asapo::Consumer::GetLast()
    /// \param[in] consumer the consumer that is acted upon
    asapoError asapoConsumerGetLast(asapoConsumer consumer,
                                    asapoMessageMeta info,
                                    asapoMessageData* data,
                                    const char* stream) {
        delete *data;
        asapo::MessageData d;
        auto err = consumer->GetLast(info, data ? &d : nullptr, stream);
        if (data) {
            *data = d.release();
        }
        return err.release();
    }

    //! wraps asapo::Consumer::GetNext()
    /// \copydoc asapo::Consumer::GetNext()
    /// \param[in] consumer the consumer that is acted upon
    asapoError asapoConsumerGetNext(asapoConsumer consumer,
                                    asapoGroupId group_id,
                                    asapoMessageMeta info,
                                    asapoMessageData* data,
                                    const char* stream) {
        delete *data;
        asapo::MessageData d;
        auto err = consumer->GetNext(*group_id, info, data ? &d : nullptr, stream);
        if (data) {
            *data = d.release();
        }
        return err.release();
    }

    //! clean up message data
    /// \param[in] data the handle of the data
    /// frees the resources occupied by data, sets *data to NULL
    void asapoDeleteMessageData(asapoMessageData* data) {
        delete *data;
        *data = nullptr;
    }

    //! give acess to data
    /// \param[in] data the handle of the data
    /// \return const char pointer to the data blob, valid until deletion or reuse of data
    const char* asapoMessageDataGetAsChars(const asapoMessageData data) {
        return reinterpret_cast<const char*>(data);
    }

    //! wraps asapo::SourceCredentials::SourceCredentials()
    /// \copydoc asapo::SourceCredentials::SourceCredentials()
    asapoSourceCredentials asapoCreateSourceCredentials(enum asapoSourceType type,
            const char* beamtime,
            const char* beamline,
            const char* data_source,
            const char* token) {
        return new asapo::SourceCredentials(static_cast<asapo::SourceType>(type),
                                            beamtime, beamline,
                                            data_source, token);
    }
    //! clean up sourceCredentials
    ///  frees the resources occupied by cred, sets *cred to NULL
    void asapoDeleteSourceCredentials(asapoSourceCredentials* cred) {
        delete *cred;
        cred = nullptr;
    }
    //! create asapoMessageMeta object
    /// create a metadata object, the handle can be used as a parameter to  consumer functions
    /// \sa asapoConsumerGetLast()
    /// \return handle to metadata object
    asapoMessageMeta asapoCreateMessageMeta() {
        return new asapo::MessageMeta;
    }
    //! clean up asapoMessageMeta object
    /// frees the resources occupied by meta, sets *meta to NULL
    void asapoDeleteMessageMeta(asapoMessageMeta* meta) {
        delete *meta;
        *meta = nullptr;
    }

    //! get name from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return pointer to the name string, valid until md is reused or deleted only!
    const char* asapoMessageMetaGetName(const asapoMessageMeta md) {
        return md->name.c_str();
    }

    //! get timestamp of the metadata object
    /// \param[in] md handle of the metadata object
    /// \param[out] stamp the timestamp as timespec
    /// \sa asapo::MessageMeta
    void asapoMessageMetaGetTimestamp(const asapoMessageMeta md,
                                      struct timespec* stamp) {
        timePointToTimeSpec(md->timestamp, stamp);
    }

    //! get size from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return size of the associated data blob
    /// \sa asapo::MessageMeta
    uint64_t asapoMessageMetaGetSize(const asapoMessageMeta md) {
        return md->size;
    }
    //! get id from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return id of the associated data blob
    /// \sa asapo::MessageMeta
    uint64_t asapoMessageMetaGetId(const asapoMessageMeta md) {
        return md->id;
    }
    //! get source from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return pointer to the source string, valid until md is reused or deleted only!
    /// \sa asapo::MessageMeta
    const char* asapoMessageMetaGetSource(const asapoMessageMeta md) {
        return md->source.c_str();
    }
    //! get metadata? from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return pointer to the metadata string, valid until md is reused or deleted only!
    /// \sa asapo::MessageMeta
    const char* asapoMessageMetaGetMetaData(const asapoMessageMeta md) {
        return md->metadata.c_str();
    }
    //! get buffer id from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return buffer id
    /// \sa asapo::MessageMeta
    uint64_t asapoMessageMetaGetBuf_id(const asapoMessageMeta md) {
        return md->buf_id;
    }
    //! get dataset substream id from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return dataset substream id
    /// \sa asapo::MessageMeta
    uint64_t asapoMessageMetaGetDataset_Substream(const asapoMessageMeta md) {
        return md->dataset_substream;
    }



    //! get last id from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return last id
    /// \sa asapo::StreamInfo
    uint64_t asapoStreamInfoGetLast_id(const asapoStreamInfo info) {
        return info->last_id;
    }
    //! get stream name from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return  pointer to the name string, valid until asapoStreamInfos object is deleted
    /// \sa asapo::StreamInfo
    const char* asapoStreamInfoGetName(const asapoStreamInfo info) {
        return info->name.c_str();
    }
    //! get finished state from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return finised state, 0 = false
    /// \sa asapo::StreamInfo
    asapoBool asapoStreamInfoGetFfinished(const asapoStreamInfo info) {
        return info->finished;
    }
    //! get next stream name? from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return  pointer to the name string, valid until asapoStreamInfos object is deleted
    /// \sa asapo::StreamInfo
    const char* asapoStreamInfoGetNext_stream(const asapoStreamInfo info) {
        return info->next_stream.c_str();
    }
    //! get creation time from the stream info object
    /// \param[in] info handle of the stream info object
    /// \param[out] stamp creation timestamp as timespec
    /// \sa asapo::StreamInfo
    void asapoStreamInfoGetTimestampCreated(const asapoStreamInfo info,
                                            struct timespec* stamp) {
        timePointToTimeSpec(info->timestamp_created, stamp);
    }
    //! get time of last entry from the stream info object
    /// \param[in] info handle of the stream info object
    /// \param[out] stamp last entry timestamp as timespec
    /// \sa asapo::StreamInfo
    void asapoStreamInfoGetTimestampLastEntry(const asapoStreamInfo info,
                                              struct timespec* stamp) {
        timePointToTimeSpec(info->timestamp_lastentry, stamp);
    }



}
