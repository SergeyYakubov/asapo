#define __CONSUMER_C_INTERFACE_IMPLEMENTATION__
#include "asapo/asapo_consumer.h"
//! boolean type
typedef bool AsapoBool;

//! handle for an asapo consumer
/// created by asapo_create_consumer()
/// delete after use with asapo_delete_consumer()
/// all operations are done vis tha asapoConsumerXxx() functions
/// \sa asapo::Consumer
typedef asapo::Consumer* AsapoConsumer;

//! handle for credentials to acess a source from a consumer
/// created by asapo_create_source_credentials()
/// delete after deletion of consumer with asapo_delete_source_credentials()
/// \sa asapo::SourceCredentials
typedef asapo::SourceCredentials* AsapoSourceCredentials;

//! handle for an asapo error
/// either a return value, NULL if no error
/// or an output parameter, then a pointer to an asapoError will be used and set to NULL or something
/// needs to be cleared after use with asapo_clear_error()
/// text version of an error: asapo_error_explain()
/// enum value of the error: asapo_error_get_type(), \sa ::asapoErrorType asapo::ConsumerErrorType
typedef asapo::ErrorInterface* AsapoError;

//! handle for metadata of a message
/// create with asapo_create_message_meta()
/// delete after use with asapo_delete_message_meta()
/// A set of getters asapoMessageMetaGetXxx() are defined
/// \sa asapo::MessageMeta
typedef asapo::MessageMeta* AsapoMessageMeta;

//! handle for set of metadata of messages
/// create with asapo_consumer_queryy_messages()
/// delete after use with asapo_delete_message_meta()
/// \sa asapo::MessageMetas
typedef asapo::MessageMetas* AsapoMessageMetas;


//! handle for data recieved by the consumer
/// set as outout parameter via asapo_consumer_get_next(), asapo_consumer_get_last()
/// delete after use with asapo_delete_message_data()
/// access to the data is granted via  asapo_message_data_get_as_chars()
typedef uint8_t* AsapoMessageData;

//! handle for string return types
/// return type of several functions
/// create with asapo_create_string()
/// delete after use with asapo_delete_string()
/// a const pointer to the content can be obtained with asapo_string_c_str()
typedef std::string* AsapoString;

//! handle for info about a stream,
/// object is deleted implicityly by  asapo_delete_stream_infos()
/// may be set via asapoStreamInfosGetInfo()
/// \sa asapo::StreamInfo asapo_stream_info_get_last_id() asapo_stream_info_get_name() asapo_stream_info_get_ffinished()  asapo_stream_info_get_next_stream()  asapo_stream_info_get_timestamp_created() asapo_stream_info_get_timestamp_last_entry()
typedef asapo::StreamInfo* AsapoStreamInfo;

//! handle for a set of stream infos
/// touch only with proper functions and use asapo_delete_stream_infos() to delete,
/// created by asapo_consumer_get_stream_list()
/// \sa asapo_delete_stream_infos() asapo_stream_infos_get_item() asapo_stream_infos_get_size()
typedef asapo::StreamInfos* AsapoStreamInfos;

//! handle for message id lists
/// touch only with proper functions and use asapo_delete_id_list() to delete,
/// created by asapo_consumer_get_unacknowledged_messages()
/// \sa asapo::IdList asapo_id_list_get_size() asapo_id_list_get_item()
typedef asapo::IdList* AsapoIdList;

//! handle for data sets
/// touch only with proper functions and use asapo_delete_data_set() to delete
typedef asapo::DataSet* AsapoDataSet;
#include <algorithm>

template <typename t> constexpr bool operator==(unsigned lhs, t rhs) {
    return lhs == static_cast<typename std::underlying_type<t>::type>(rhs);
}

#define dataGetterStart \
	if (data) delete *data; \
	asapo::MessageData d; \
	asapo::MessageMeta* fi = info ? new asapo::MessageMeta : nullptr; \
	if (info) { \
    delete *info; \
    } \

#define dataGetterStop \
    if (data) { \
        *data = d.release(); \
    } \
    if (info) { \
         *info = fi;\
    } \
    return err.release();\

extern "C" {
#include "asapo/consumer_c.h"
    static_assert(kNoData == asapo::ConsumerErrorType::kNoData&&
                  kEndOfStream == asapo::ConsumerErrorType::kEndOfStream&&
                  kStreamFinished == asapo::ConsumerErrorType::kStreamFinished&&
                  kUnavailableService == asapo::ConsumerErrorType::kUnavailableService&&
                  kInterruptedTransaction == asapo::ConsumerErrorType::kInterruptedTransaction&&
                  kLocalIOError == asapo::ConsumerErrorType::kLocalIOError&&
                  kWrongInput == asapo::ConsumerErrorType::kWrongInput&&
                  kPartialData == asapo::ConsumerErrorType::kPartialData&&
                  kUnsupportedClient == asapo::ConsumerErrorType::kUnsupportedClient,
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

    static void time_point_to_time_spec(std::chrono::system_clock::time_point tp,
                                        struct timespec* stamp) {
        stamp->tv_sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
        stamp->tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count() % 1000000000;
    }

    /// \copydoc asapo::ErrorInterface::Explain()
    /// \param[out] buf will be filled with the explanation
    /// \param[in] maxSize max size of buf in bytes
    void asapo_error_explain(const AsapoError error, char* buf, size_t maxSize) {
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

    enum AsapoConsumerErrorType asapo_error_get_type(const AsapoError error) {
        auto consumer_err =
            dynamic_cast<const asapo::ServiceError<asapo::ConsumerErrorType, asapo::ErrorType::kConsumerError>*>(error);
        if (consumer_err != nullptr) {
            return static_cast<AsapoConsumerErrorType>(consumer_err->GetServiceErrorType());
        } else {
            return kUnknownError;
        }
    }
    //! clean up error
    /// frees the resources occupied by error,
    /// sets *error to NULL
    void asapo_clear_error(AsapoError* error) {
        if ( *error == nullptr) {
            return;
        }
        delete *error;
        *error = nullptr;
    }


    //! creata a consumer
    /// \copydoc asapo::ConsumerFactory::CreateConsumer
    /// return handle to the created cosumer
    AsapoConsumer asapo_create_consumer(const char* server_name,
                                        const char* source_path,
                                        AsapoBool has_filesysytem,
                                        AsapoSourceCredentials source,
                                        AsapoError* error) {


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
    void asapo_delete_consumer(AsapoConsumer* consumer) {
        delete *consumer;
        *consumer = nullptr;
    }
    //! wraps asapo::Consumer::GenerateNewGroupId()
    /// \copydoc asapo::Consumer::GenerateNewGroupId()
    /// \param[in] consumer the handle of the consumer concerned
    AsapoString asapo_consumer_generate_new_group_id(AsapoConsumer consumer,
                                                     AsapoError* error) {
        asapo::Error err;
        auto result = new std::string(consumer->GenerateNewGroupId(&err));
        *error = err.release();
        return result;
    }
    //! create an asapoString from a c string
    /// \param[in] content the characters that will make up the new string
    /// \return handle of the new asapo string, delete after use with asapo_delete_string()
    AsapoString asapo_create_string(const char* content) {
        return new std::string(content);
    }
    //! append to an exising asapo string
    /// \param[in] str the handle of the asapoString in question
    /// \param[in] content the c string with the characters to append
    void asapo_string_append(AsapoString str, const char* content) {
        *str += content;
    }
    //! give a pointer to the content of the asapoString
    /// \param[in] str the handle of the asapoString in question
    /// \return const char pointer to the content
    const char* asapo_string_c_str(const AsapoString str) {
        return str->c_str();
    }
    //! give the size of an asapoString
    /// \param[in] str the handle of the asapoString in question
    /// \return the number of bytes in the string , not counting the final nul byte.
    size_t asapo_string_size(const AsapoString str) {
        return str->size();
    }
    //! clean up string
    /// frees the resources occupied by str, sets *str to NULL
/// \param[in] str the handle of the asapoString in question
    void asapo_delete_string(AsapoString* str) {
        delete *str;
        *str = nullptr;
    }

    //! wraps asapo::Consumer::SetTimeout()
    /// \copydoc asapo::Consumer::SetTimeout()
    /// \param[in] consumer the handle of the consumer concerned
    void asapo_consumer_set_timeout(AsapoConsumer consumer, uint64_t timeout_ms) {
        consumer->SetTimeout(timeout_ms);
    }

    //! wraps asapo::Consumer::ResetLastReadMarker()
    /// \copydoc asapo::Consumer::ResetLastReadMarker()
    /// \param[in] consumer the handle of the consumer concerned
    AsapoError asapo_consumer_reset_last_read_marker(AsapoConsumer consumer,
                                                     const AsapoString group_id,
                                                     const char* stream) {
        auto err = consumer->ResetLastReadMarker(*group_id, stream);
        return err.release();
    }

    //! wraps asapo::Consumer::SetLastReadMarker()
    /// \copydoc asapo::Consumer::SetLastReadMarker()
    /// \param[in] consumer the handle of the consumer concerned
    AsapoError asapo_consumer_set_last_read_marker(AsapoConsumer consumer,
                                                   const AsapoString group_id,
                                                   uint64_t value,
                                                   const char* stream) {
        auto err = consumer->SetLastReadMarker(*group_id, value, stream);
        return err.release();
    }
    //! wraps asapo::Consumer::Acknowledge()
    /// \copydoc asapo::Consumer::Acknowledge()
    /// \param[in] consumer the handle of the consumer concerned
    AsapoError asapo_consumer_acknowledge(AsapoConsumer consumer,
                                          const AsapoString group_id,
                                          uint64_t id,
                                          const char* stream) {
        auto err = consumer->Acknowledge(*group_id, id, stream);
        return err.release();
    }
    //! wraps asapo::Consumer::NegativeAcknowledge()
    /// \copydoc asapo::Consumer::NegativeAcknowledge()
    /// \param[in] consumer the handle of the consumer concerned
    AsapoError asapo_consumer_negative_acknowledge(AsapoConsumer consumer,
                                                   const AsapoString group_id,
                                                   uint64_t id,
                                                   uint64_t delay_ms,
                                                   const char* stream) {
        auto err = consumer->NegativeAcknowledge(*group_id, id, delay_ms, stream);
        return err.release();
    }

    //! wraps asapo::Consumer::GetUnacknowledgedMessages()
    /// \copydoc asapo::Consumer::GetUnacknowledgedMessages()
    /// \param[in] consumer the handle of the consumer concerned
    AsapoIdList asapo_consumer_get_unacknowledged_messages(AsapoConsumer consumer,
            AsapoString group_id,
            uint64_t from_id,
            uint64_t to_id,
            const char* stream,
            AsapoError* error) {
        asapo::Error err;
        auto list = new asapo::IdList(consumer->GetUnacknowledgedMessages(*group_id,
                                      from_id, to_id,
                                      stream,
                                      &err));
        *error = err.release();
        return list;
    }
    //! cleans up an IdList
    void asapo_delete_id_list(AsapoIdList* list) {
        delete *list;
        *list = nullptr;
    }
    //! give number of items in an id list
    /// \param[in] list handle of an id list
    size_t asapo_id_list_get_size(const AsapoIdList list) {
        return list->size();
    }
    //! give one items from an id list
    /// \param[in] list handle of an id list
    /// \param[in] index index of the item to return, start at 0
    uint64_t asapo_id_list_get_item(const AsapoIdList list,
                                    size_t index) {
        return list->at(index);
    }
    //! wraps asapo::Consumer::ForceNoRdma()
    /// \copydoc asapo::Consumer::ForceNoRdma()
    /// \param[in] consumer the handle of the consumer concerned
    void asapo_consumer_force_no_rdma(AsapoConsumer consumer);
    //! wraps asapo::Consumer::CurrentConnectionType()
    /// \copydoc asapo::Consumer::CurrentConnectionType()
    /// \param[in] consumer the handle of the consumer concerned
    enum AsapoNetworkConnectionType asapo_consumer_current_connection_type(AsapoConsumer consumer);


    //! get list of streams, wraps asapo::Consumer::GetStreamList()
    /*!
      \param[in] consumer the consumer that is acted upon
      \param[in] from whatever
      \param[in] filter select the kind of stream
      \param[out] error will contain a pointer to an asapoError if a problem occured, NULL else.
      \return object that contains the stream infos
      \sa asapoStreamInfosGetInfo() asapo_stream_infos_get_size() asapo_delete_stream_infos()
    */
    AsapoStreamInfos asapo_consumer_get_stream_list(AsapoConsumer consumer,
                                                    const char* from,
                                                    enum AsapoStreamFilter filter,
                                                    AsapoError* error) {
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
    const AsapoStreamInfo asapo_stream_infos_get_item(const AsapoStreamInfos infos,
                                                      size_t index) {
        return &infos->at(index);
    }
    //! get size (number of elements) of a stream infos handle
    /// \param[in] infos handle for stream infos
    /// \return number of elements in the handle
    size_t asapo_stream_infos_get_size(const AsapoStreamInfos infos) {
        return infos->size();
    }
    //! clean up streamInfos
    /// frees the resources occupied by infos, sets *infos to NULL

    void asapo_delete_stream_infos(AsapoStreamInfos* infos) {
        delete *infos;
        *infos = nullptr;
    }

    //! wraps asapo::Consumer::DeleteStream()
    /// \copydoc asapo::Consumer::DeleteStream()
    /// \param[in] consumer the consumer that is acted upon
    /// \param[in] delete_meta the delete_meta part of the asapo::DeleteStreamOptions
    /// \param[in] error_on_not_exist the error_on_not_exist part of the asapo::DeleteStreamOptions
    AsapoError asapo_consumer_delete_stream(AsapoConsumer consumer,
                                            const char* stream,
                                            AsapoBool delete_meta,
                                            AsapoBool error_on_not_exist) {
        asapo::DeleteStreamOptions opt(delete_meta, error_on_not_exist);
        auto err = consumer->DeleteStream(stream, opt);
        return err.release();
    }
    //! wraps asapo::Consumer::GetCurrentSize()
    /// \copydoc asapo::Consumer::GetCurrentSize()
    /// \param[in] consumer the consumer that is acted upon
    uint64_t asapo_consumer_get_current_size(AsapoConsumer consumer,
                                             const char* stream,
                                             AsapoError* error) {
        asapo::Error err;
        auto retval = consumer->GetCurrentSize(stream, &err);
        *error = err.release();
        return retval;
    }
    //! wraps asapo::Consumer::GetCurrentDatasetCount()
    /// \copydoc asapo::Copydoc::GetCurrentDatasetCount()
    /// \param[in] consumer the consumer that is acted upon
    uint64_t asapo_consumer_get_current_dataset_count(AsapoConsumer consumer,
                                                      const char* stream,
                                                      AsapoBool include_incomplete,
                                                      AsapoError* error) {
        asapo::Error err;
        auto retval = consumer->GetCurrentDatasetCount(stream, include_incomplete, &err);
        *error = err.release();
        return retval;
    }

    //! wraps asapo::Consumer::GetBeamtimeMeta()
    /// \copydoc asapo::Consumer::GetBeamtimeMeta()
    /// \param[in] consumer the consumer that is acted upon
    /// the returned string must be freed after use with asapo_delete_string()
    AsapoString asapo_consumer_get_beamtime_meta(AsapoConsumer consumer,
                                                 AsapoError* error) {
        asapo::Error err;
        auto retval = new std::string(consumer->GetBeamtimeMeta(&err));
        *error = err.release();
        return retval;
    }


    //! wraps asapo::Consumer::RetrieveData()
    /// \copydoc asapo::Consumer::RetrieveData()
    /// \param[in] consumer the consumer that is acted upon
    /// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
    AsapoError asapo_consumer_retrieve_data(AsapoConsumer consumer,
                                            AsapoMessageMeta* info,
                                            AsapoMessageData* data) {
        if (data) delete *data;
        asapo::MessageData d;
        auto err = consumer->RetrieveData(static_cast<asapo::MessageMeta*>(*info), data ? &d : nullptr);
        if (data) {
            *data = d.release();
        }
        return err.release();
    }

    //! wraps asapo::Consumer::GetNextDataset()
    /// \copydoc asapo::Consumer::GetNextDataset()
    /// \param[in] consumer the consumer that is acted upon
    /// the returned data set must be freed with asapo_delete_data_set() after use.
    AsapoDataSet asapo_consumer_get_next_data_set(AsapoConsumer consumer,
                                                  AsapoString group_id,
                                                  uint64_t min_size,
                                                  const char* stream,
                                                  AsapoError* error) {
        asapo::Error err;
        auto retval =  new asapo::DataSet(consumer->GetNextDataset(*group_id, min_size, stream, &err));
        *error = err.release();
        return retval;
    }

    //! wraps asapo::Consumer::GetLastDataset()
    /// \copydoc asapo::Consumer::GetLastDataset()
    /// \param[in] consumer the consumer that is acted upon
    /// the returned data set must be freed with asapo_delete_data_set() after use.
    AsapoDataSet asapo_consumer_get_last_data_set(AsapoConsumer consumer,
                                                  uint64_t min_size,
                                                  const char* stream,
                                                  AsapoError* error) {
        asapo::Error err;
        auto retval =  new asapo::DataSet(consumer->GetLastDataset(min_size, stream, &err));
        *error = err.release();
        return retval;
    }

//! wraps asapo::Consumer::GetLastAcknowledgedMessage()
/// \copydoc asapo::Consumer::GetLastAcknowledgedMessage()
/// \param[in] consumer the consumer that is acted upon
    uint64_t asapo_consumer_get_last_acknowledged_message(AsapoConsumer consumer,
            AsapoString group_id,
            const char* stream,
            AsapoError* error) {
        asapo::Error err;
        auto retval =  consumer->GetLastAcknowledgedMessage(*group_id, stream, &err);
        *error = err.release();
        return retval;
    }


//! wraps asapo::Consumer::GetDatasetById()
    /// \copydoc asapo::Consumer::GetDatasetById()
    /// \param[in] consumer the consumer that is acted upon
    /// the returned data set must be freed with asapo_delete_data_set() after use.
    AsapoDataSet asapo_consumer_get_data_set_by_id(AsapoConsumer consumer,
                                                   uint64_t id,
                                                   uint64_t min_size,
                                                   const char* stream,
                                                   AsapoError* error) {
        asapo::Error err;
        auto retval =  new asapo::DataSet(consumer->GetDatasetById(id, min_size, stream, &err));
        *error = err.release();
        return retval;
    }

    //! wraps aspao::Consumer::GetById()
    /// \copydoc aspao::Consumer::GetById()
    /// \param[in] consumer the consumer that is acted upon
    /// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
    AsapoError asapo_consumer_get_by_id(AsapoConsumer consumer,
                                        uint64_t id,
                                        AsapoMessageMeta* info,
                                        AsapoMessageData* data,
                                        const char* stream) {
        dataGetterStart;
        auto err = consumer->GetById(id, fi, data ? &d : nullptr, stream);
        dataGetterStop;
    }

    //! wraps asapo::Consumer::GetLast()
    /// \copydoc asapo::Consumer::GetLast()
    /// \param[in] consumer the consumer that is acted upon
    /// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
    AsapoError asapo_consumer_get_last(AsapoConsumer consumer,
                                       AsapoMessageMeta* info,
                                       AsapoMessageData* data,
                                       const char* stream) {
        dataGetterStart;
        auto err = consumer->GetLast(fi, data ? &d : nullptr, stream);
        dataGetterStop;
    }

    //! wraps asapo::Consumer::GetNext()
    /// \copydoc asapo::Consumer::GetNext()
    /// \param[in] consumer the consumer that is acted upon
    /// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
    AsapoError asapo_consumer_get_next(AsapoConsumer consumer,
                                       AsapoString group_id,
                                       AsapoMessageMeta* info,
                                       AsapoMessageData* data,
                                       const char* stream) {
        dataGetterStart;
        auto err = consumer->GetNext(*group_id, fi, data ? &d : nullptr, stream);
        dataGetterStop;
    }

    //! wraps asapo::Consumer::QueryMessages()
    /// \copydoc  asapo::Consumer::QueryMessages()
    /// \param[in] consumer the consumer that is acted upon
    /// the returned list must be freed with asapo_delete_message_metas() after use.
    AsapoMessageMetas asapo_consumer_query_messages(AsapoConsumer consumer,
                                                    const char* query,
                                                    const char* stream,
                                                    AsapoError* error) {
        asapo::Error err;
        auto retval = new asapo::MessageMetas(consumer->QueryMessages(query, stream, &err));
        *error = err.release();
        return retval;
    }

    //! wraps aspao::Consumer::SetResendNacs()
    /// \copydoc aspao::Consumer::SetResendNacs()
    /// \param[in] consumer the consumer that is acted upon
    void asapo_consumer_set_resend_nacs(AsapoConsumer consumer,
                                        AsapoBool resend,
                                        uint64_t delay_ms,
                                        uint64_t resend_attempts) {
        consumer->SetResendNacs(resend, delay_ms, resend_attempts);
    }


    //! clean up message data
    /// \param[in] data the handle of the data
    /// frees the resources occupied by data, sets *data to NULL
    void asapo_delete_message_data(AsapoMessageData* data) {
        delete *data;
        *data = nullptr;
    }

    //! give acess to data
    /// \param[in] data the handle of the data
    /// \return const char pointer to the data blob, valid until deletion or reuse of data
    const char* asapo_message_data_get_as_chars(const AsapoMessageData data) {
        return reinterpret_cast<const char*>(data);
    }

    //! wraps asapo::SourceCredentials::SourceCredentials()
    /// \copydoc asapo::SourceCredentials::SourceCredentials()
    AsapoSourceCredentials asapo_create_source_credentials(enum AsapoSourceType type,
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
    void asapo_delete_source_credentials(AsapoSourceCredentials* cred) {
        delete *cred;
        cred = nullptr;
    }

    //! clean up asapoMessageMeta object
    /// frees the resources occupied by meta, sets *meta to NULL
    void asapo_delete_message_meta(AsapoMessageMeta* meta) {
        delete *meta;
        *meta = nullptr;
    }

    //! get name from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return pointer to the name string, valid until md is reused or deleted only!
    const char* asapo_message_meta_get_name(const AsapoMessageMeta md) {
        return md->name.c_str();
    }

    //! get timestamp of the metadata object
    /// \param[in] md handle of the metadata object
    /// \param[out] stamp the timestamp as timespec
    /// \sa asapo::MessageMeta
    void asapo_message_meta_get_timestamp(const AsapoMessageMeta md,
                                          struct timespec* stamp) {
        time_point_to_time_spec(md->timestamp, stamp);
    }

    //! get size from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return size of the associated data blob
    /// \sa asapo::MessageMeta
    uint64_t asapo_message_meta_get_size(const AsapoMessageMeta md) {
        return md->size;
    }
    //! get id from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return id of the associated data blob
    /// \sa asapo::MessageMeta
    uint64_t asapo_message_meta_get_id(const AsapoMessageMeta md) {
        return md->id;
    }
    //! get source from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return pointer to the source string, valid until md is reused or deleted only!
    /// \sa asapo::MessageMeta
    const char* asapo_message_meta_get_source(const AsapoMessageMeta md) {
        return md->source.c_str();
    }
    //! get metadata? from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return pointer to the metadata string, valid until md is reused or deleted only!
    /// \sa asapo::MessageMeta
    const char* asapo_message_meta_get_metadata(const AsapoMessageMeta md) {
        return md->metadata.c_str();
    }
    //! get buffer id from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return buffer id
    /// \sa asapo::MessageMeta
    uint64_t asapo_message_meta_get_buf_id(const AsapoMessageMeta md) {
        return md->buf_id;
    }
    //! get dataset substream id from the metadata object
    /// \param[in] md handle of the metadata object
    /// \return dataset substream id
    /// \sa asapo::MessageMeta
    uint64_t asapo_message_meta_get_dataset_substream(const AsapoMessageMeta md) {
        return md->dataset_substream;
    }

    //! get last id from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return last id
    /// \sa asapo::StreamInfo
    uint64_t asapo_stream_info_get_last_id(const AsapoStreamInfo info) {
        return info->last_id;
    }
    //! get stream name from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return  pointer to the name string, valid until asapoStreamInfos object is deleted
    /// \sa asapo::StreamInfo
    const char* asapo_stream_info_get_name(const AsapoStreamInfo info) {
        return info->name.c_str();
    }
    //! get finished state from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return finised state, 0 = false
    /// \sa asapo::StreamInfo
    AsapoBool asapo_stream_info_get_ffinished(const AsapoStreamInfo info) {
        return info->finished;
    }
    //! get next stream name? from the stream info object
    /// \param[in] info handle of the stream info object
    /// \return  pointer to the name string, valid until asapoStreamInfos object is deleted
    /// \sa asapo::StreamInfo
    const char* asapo_stream_info_get_next_stream(const AsapoStreamInfo info) {
        return info->next_stream.c_str();
    }
    //! get creation time from the stream info object
    /// \param[in] info handle of the stream info object
    /// \param[out] stamp creation timestamp as timespec
    /// \sa asapo::StreamInfo
    void asapo_stream_info_get_timestamp_created(const AsapoStreamInfo info,
                                                 struct timespec* stamp) {
        time_point_to_time_spec(info->timestamp_created, stamp);
    }
    //! get time of last entry from the stream info object
    /// \param[in] info handle of the stream info object
    /// \param[out] stamp last entry timestamp as timespec
    /// \sa asapo::StreamInfo
    void asapo_stream_info_get_timestamp_last_entry(const AsapoStreamInfo info,
                                                    struct timespec* stamp) {
        time_point_to_time_spec(info->timestamp_lastentry, stamp);
    }

    //! clean up AsapoDataSet object
    /// frees the resources occupied by set, sets *set to NULL
    void asapo_delete_data_set(AsapoDataSet* set) {
        delete *set;
        *set = nullptr;
    }
    //! get id from data set object
    /// \param[in] set handle of the data set object
    /// \return id of the data set
    /// \sa asapo::DataSet
    uint64_t asapo_data_set_get_id(const AsapoDataSet set) {
        return set->id;
    }
    //! get ecpected size from data set object
    /// \param[in] set handle of the data set object
    /// \return expected size of the data set
    /// \sa asapo::DataSet
    uint64_t asapo_data_set_get_expected_size(const AsapoDataSet set) {
        return set->expected_size;
    }
    //! get number of message meta objects from data set object
    /// \param[in] set handle of the data set object
    /// \return number of message meta objects of the data set
    /// \sa asapo::DataSet
    size_t asapo_data_set_get_size(const AsapoDataSet set) {
        return set->content.size();
    }
    //! get one message meta object handle from data set object
    /// \param[in] set handle of the data set object
    /// \param[in] index of the message meta object wanted
    /// \return handle of the meta data object, do not delete!
    /// \sa asapo::DataSet
    const AsapoMessageMeta asapo_data_set_get_item(const AsapoDataSet set,
                                                   size_t index) {
        return &(set->content.at(index));
    }

//! clean up message metas object
    /// frees the resources occupied by metas, sets *metas to NULL
    void asapo_delete_message_metas(AsapoMessageMetas* metas) {
        delete *metas;
        *metas = nullptr;
    }
    //! get number of message meta objects from metas object
    /// \param[in] metas handle of the metas object
    /// \return number of message meta objects of the data set
    /// \sa asapo::MessageMetas
    size_t asapo_message_metas_get_size(const AsapoMessageMetas metas) {
        return metas->size();
    }
    //! get one message meta object handle from metas object
    /// \param[in] metas handle of the metas object
    /// \param[in] index of the message meta object wanted
    /// \return handle of the meta data object, do not delete!
    /// \sa asapo::MessageMetas
    const AsapoMessageMeta asapo_message_metas_get_item(const AsapoMessageMetas metas,
            size_t index) {
        return &(metas->at(index));
    }

}
