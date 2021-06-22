#define __CONSUMER_C_INTERFACE_IMPLEMENTATION__
#include "asapo/asapo_consumer.h"
//! boolean type
typedef bool AsapoBool;

class AsapoHandle {
 public:
  virtual void Destroy() = 0;
  virtual ~AsapoHandle() {};
};

template<class T>
class AsapoHandlerHolder final : public AsapoHandle {
 public:
  AsapoHandlerHolder(bool manage_memory = true) : handle{nullptr}, manage_memory_{manage_memory} {};
  AsapoHandlerHolder(T *handle_i, bool manage_memory = true) : handle{handle_i}, manage_memory_{manage_memory} {};
  void Destroy() override {
      if (handle && manage_memory_) {
          handle = nullptr;
      }
  };
  ~AsapoHandlerHolder() override {
      if (!manage_memory_) {
          handle.release();
      }
  }
  std::unique_ptr<T> handle{nullptr};
 protected:
  bool manage_memory_{true};
};

//! handle for an asapo consumer
/// created by asapo_create_consumer()
/// delete after use with asapo_delete_consumer()
/// all operations are done vis tha asapoConsumerXxx() functions
/// \sa asapo::Consumer
typedef AsapoHandlerHolder<asapo::Consumer> *AsapoConsumerHandle;

//! handle for credentials to acess a source from a consumer
/// created by asapo_create_source_credentials()
/// delete after deletion of consumer with asapo_delete_source_credentials()
/// \sa asapo::SourceCredentials
typedef AsapoHandlerHolder<asapo::SourceCredentials> *AsapoSourceCredentialsHandle;

//! handle for an asapo error
/// either a return value, NULL if no error
/// or an output parameter, then a pointer to an asapoError will be used and set to NULL or something
/// needs to be cleared after use with asapo_clear_error()
/// text version of an error: asapo_error_explain()
/// enum value of the error: asapo_error_get_type(), \sa ::asapoErrorType asapo::ConsumerErrorType
typedef AsapoHandlerHolder<asapo::ErrorInterface> *AsapoErrorHandle;

//! handle for metadata of a message
/// create with asapo_create_message_meta()
/// delete after use with asapo_delete_message_meta()
/// A set of getters asapoMessageMetaGetXxx() are defined
/// \sa asapo::MessageMeta
typedef AsapoHandlerHolder<asapo::MessageMeta> *AsapoMessageMetaHandle;

//! handle for set of metadata of messages
/// create with asapo_consumer_queryy_messages()
/// delete after use with asapo_delete_message_meta()
/// \sa asapo::MessageMetas
typedef AsapoHandlerHolder<asapo::MessageMetas> *AsapoMessageMetasHandle;

//! handle for data recieved by the consumer
/// set as outout parameter via asapo_consumer_get_next(), asapo_consumer_get_last()
/// delete after use with asapo_delete_message_data()
/// access to the data is granted via  asapo_message_data_get_as_chars()
typedef AsapoHandlerHolder<uint8_t[]> *AsapoMessageDataHandle;

//! handle for string return types
/// return type of several functions
/// create with asapo_create_string()
/// delete after use with asapo_delete_string()
/// a const pointer to the content can be obtained with asapo_string_c_str()
typedef AsapoHandlerHolder<std::string> *AsapoStringHandle;

//! handle for info about a stream,
/// object is deleted implicityly by  asapo_delete_stream_infos()
/// may be set via asapoStreamInfosGetInfo()
/// \sa asapo::StreamInfo asapo_stream_info_get_last_id() asapo_stream_info_get_name() asapo_stream_info_get_ffinished()  asapo_stream_info_get_next_stream()  asapo_stream_info_get_timestamp_created() asapo_stream_info_get_timestamp_last_entry()
typedef AsapoHandlerHolder<asapo::StreamInfo> *AsapoStreamInfoHandle;

//! handle for a set of stream infos
/// touch only with proper functions and use asapo_delete_stream_infos() to delete,
/// created by asapo_consumer_get_stream_list()
/// \sa asapo_delete_stream_infos() asapo_stream_infos_get_item() asapo_stream_infos_get_size()
typedef AsapoHandlerHolder<asapo::StreamInfos> *AsapoStreamInfosHandle;

//! handle for message id lists
/// touch only with proper functions and use asapo_delete_id_list() to delete,
/// created by asapo_consumer_get_unacknowledged_messages()
/// \sa asapo::IdList asapo_id_list_get_size() asapo_id_list_get_item()
typedef AsapoHandlerHolder<asapo::IdList> *AsapoIdListHandle;

//! handle for data sets
/// touch only with proper functions and use asapo_delete_data_set() to delete
typedef AsapoHandlerHolder<asapo::DataSet> *AsapoDataSetHandle;
#include <algorithm>

template<typename t>
constexpr bool operator==(unsigned lhs, t rhs) {
    return lhs == static_cast<typename std::underlying_type<t>::type>(rhs);
}

void process_error(AsapoErrorHandle *error, asapo::Error err) {
    if (error == nullptr) {
        return;
    }
    if (*error == nullptr) {
        *error = new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
        return;
    }
    (*error)->handle = std::move(err);
}

#define dataGetterStart \
    asapo::MessageData d; \
    asapo::MessageMeta* fi = info ? new asapo::MessageMeta : nullptr; \

#define dataGetterStop \
    if (data) { \
        if (*data == nullptr) { \
            *data = new AsapoHandlerHolder<uint8_t[]>{}; \
        } \
            (*data)->handle.reset(d.release());\
    } \
    if (info) {\
        if (*info == nullptr) {\
            *info = new AsapoHandlerHolder<asapo::MessageMeta>{nullptr};\
        } \
        (*info)->handle.reset(fi); \
    } \

extern "C" {
#include "asapo/consumer_c.h"
static_assert(kNoData == asapo::ConsumerErrorType::kNoData &&
                  kEndOfStream == asapo::ConsumerErrorType::kEndOfStream &&
                  kStreamFinished == asapo::ConsumerErrorType::kStreamFinished &&
                  kUnavailableService == asapo::ConsumerErrorType::kUnavailableService &&
                  kInterruptedTransaction == asapo::ConsumerErrorType::kInterruptedTransaction &&
                  kLocalIOError == asapo::ConsumerErrorType::kLocalIOError &&
                  kWrongInput == asapo::ConsumerErrorType::kWrongInput &&
                  kPartialData == asapo::ConsumerErrorType::kPartialData &&
                  kUnsupportedClient == asapo::ConsumerErrorType::kUnsupportedClient,
              "incompatible bit reps between c++ and c for asapo::ErrorType");
static_assert(kAllStreams == asapo::StreamFilter::kAllStreams &&
                  kFinishedStreams == asapo::StreamFilter::kFinishedStreams &&
                  kUnfinishedStreams == asapo::StreamFilter::kUnfinishedStreams,
              "incompatible bit reps between c++ and c for asapo::StreamFilter");
static_assert(kProcessed == asapo::SourceType::kProcessed &&
                  kRaw == asapo::SourceType::kRaw,
              "incompatible bit reps between c++ and c for asapo::SourceType");
static_assert(kUndefined == asapo::NetworkConnectionType::kUndefined &&
                  kAsapoTcp == asapo::NetworkConnectionType::kAsapoTcp &&
                  kFabric == asapo::NetworkConnectionType::kFabric,
              "incompatible bit reps between c++ and c for asapo::NetworkConnectionType");

static void time_point_to_time_spec(std::chrono::system_clock::time_point tp,
                                    struct timespec *stamp) {
    stamp->tv_sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    stamp->tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count() % 1000000000;
}

AsapoBool asapo_is_error(AsapoErrorHandle err) {
    return err!=nullptr && err->handle != nullptr;
}

/// \copydoc asapo::ErrorInterface::Explain()
/// \param[out] buf will be filled with the explanation
/// \param[in] maxSize max size of buf in bytes
void asapo_error_explain(const AsapoErrorHandle error, char *buf, size_t maxSize) {
    if (error->handle) {
        strncpy(buf,error->handle->Explain().c_str(),maxSize-1);
        buf[maxSize] = '\0';
    } else {
        static std::string msg("no error");
        std::copy_n(msg.begin(), std::max(msg.size(), maxSize), buf);
        buf[std::max(maxSize - 1, msg.size())] = '\0';
    }
}

enum AsapoConsumerErrorType asapo_error_get_type(const AsapoErrorHandle error) {
    auto consumer_err =
        dynamic_cast<const asapo::ServiceError<asapo::ConsumerErrorType, asapo::ErrorType::kConsumerError> *>(error->handle.get());
    if (consumer_err != nullptr) {
        return static_cast<AsapoConsumerErrorType>(consumer_err->GetServiceErrorType());
    } else {
        return kUnknownError;
    }
}
//! clean up error
/// frees the resources occupied by error,
/// sets *error to NULL
void asapo_clear_error(AsapoErrorHandle *error) {
    asapo_free_handle((void **) error);
}

//! creata a consumer
/// \copydoc asapo::ConsumerFactory::CreateConsumer
/// return handle to the created cosumer
AsapoConsumerHandle asapo_create_consumer(const char *server_name,
                                          const char *source_path,
                                          AsapoBool has_filesysytem,
                                          AsapoSourceCredentialsHandle source,
                                          AsapoErrorHandle *error) {

    asapo::Error err;
    auto c = asapo::ConsumerFactory::CreateConsumer(server_name,
                                                    source_path,
                                                    has_filesysytem,
                                                    *(source->handle),
                                                    &err);
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<asapo::Consumer>(c.release());
}

//! clean up consumer
/// frees the resources occupied by consumer, sets *consumer to NULL
/// \param[in] consumer the handle of the consumer concerned
void asapo_delete_consumer(AsapoConsumerHandle *consumer) {
//        delete *consumer;
//        *consumer = nullptr;
}

//! wraps asapo::Consumer::GenerateNewGroupId()
/// \copydoc asapo::Consumer::GenerateNewGroupId()
/// \param[in] consumer the handle of the consumer concerned
AsapoStringHandle asapo_consumer_generate_new_group_id(AsapoConsumerHandle consumer,
                                                       AsapoErrorHandle *error) {
    asapo::Error err;
    auto result = new std::string(consumer->handle->GenerateNewGroupId(&err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<std::string>{result};
}
//! create an asapoString from a c string
/// \param[in] content the characters that will make up the new string
/// \return handle of the new asapo string, delete after use with asapo_delete_string()
AsapoStringHandle asapo_create_string(const char *content) {
    return new AsapoHandlerHolder<std::string>{new std::string(content)};
}
//! append to an exising asapo string
/// \param[in] str the handle of the asapoString in question
/// \param[in] content the c string with the characters to append
void asapo_string_append(AsapoStringHandle str, const char *content) {
    str->handle->append(content);
}
//! give a pointer to the content of the asapoString
/// \param[in] str the handle of the asapoString in question
/// \return const char pointer to the content
const char *asapo_string_c_str(const AsapoStringHandle str) {
    return str->handle->c_str();
}
//! give the size of an asapoString
/// \param[in] str the handle of the asapoString in question
/// \return the number of bytes in the string , not counting the final nul byte.
size_t asapo_string_size(const AsapoStringHandle str) {
    return str->handle->size();
}
//! clean up string
/// frees the resources occupied by str, sets *str to NULL
/// \param[in] str the handle of the asapoString in question
void asapo_delete_string(AsapoStringHandle *str) {
    delete *str;
    *str = nullptr;
}

//! wraps asapo::Consumer::SetTimeout()
/// \copydoc asapo::Consumer::SetTimeout()
/// \param[in] consumer the handle of the consumer concerned
void asapo_consumer_set_timeout(AsapoConsumerHandle consumer, uint64_t timeout_ms) {
    consumer->handle->SetTimeout(timeout_ms);
}

//! wraps asapo::Consumer::ResetLastReadMarker()
/// \copydoc asapo::Consumer::ResetLastReadMarker()
/// \param[in] consumer the handle of the consumer concerned
AsapoErrorHandle asapo_consumer_reset_last_read_marker(AsapoConsumerHandle consumer,
                                                       const AsapoStringHandle group_id,
                                                       const char *stream) {
    auto err = consumer->handle->ResetLastReadMarker(*group_id->handle, stream);
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}

//! wraps asapo::Consumer::SetLastReadMarker()
/// \copydoc asapo::Consumer::SetLastReadMarker()
/// \param[in] consumer the handle of the consumer concerned
AsapoErrorHandle asapo_consumer_set_last_read_marker(AsapoConsumerHandle consumer,
                                                     const AsapoStringHandle group_id,
                                                     uint64_t value,
                                                     const char *stream) {
    auto err = consumer->handle->SetLastReadMarker(*group_id->handle, value, stream);
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}
//! wraps asapo::Consumer::Acknowledge()
/// \copydoc asapo::Consumer::Acknowledge()
/// \param[in] consumer the handle of the consumer concerned
AsapoErrorHandle asapo_consumer_acknowledge(AsapoConsumerHandle consumer,
                                            const AsapoStringHandle group_id,
                                            uint64_t id,
                                            const char *stream) {
    auto err = consumer->handle->Acknowledge(*group_id->handle, id, stream);
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}
//! wraps asapo::Consumer::NegativeAcknowledge()
/// \copydoc asapo::Consumer::NegativeAcknowledge()
/// \param[in] consumer the handle of the consumer concerned
AsapoErrorHandle asapo_consumer_negative_acknowledge(AsapoConsumerHandle consumer,
                                                     const AsapoStringHandle group_id,
                                                     uint64_t id,
                                                     uint64_t delay_ms,
                                                     const char *stream) {
    auto err = consumer->handle->NegativeAcknowledge(*group_id->handle, id, delay_ms, stream);
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}

//! wraps asapo::Consumer::GetUnacknowledgedMessages()
/// \copydoc asapo::Consumer::GetUnacknowledgedMessages()
/// \param[in] consumer the handle of the consumer concerned
AsapoIdListHandle asapo_consumer_get_unacknowledged_messages(AsapoConsumerHandle consumer,
                                                             AsapoStringHandle group_id,
                                                             uint64_t from_id,
                                                             uint64_t to_id,
                                                             const char *stream,
                                                             AsapoErrorHandle *error) {
    asapo::Error err;
    auto list = new asapo::IdList(consumer->handle->GetUnacknowledgedMessages(*group_id->handle,
                                                                              from_id, to_id,
                                                                              stream,
                                                                              &err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<asapo::IdList>{list};
}
//! cleans up an IdList
void asapo_delete_id_list(AsapoIdListHandle *list) {
    delete *list;
    *list = nullptr;
}
//! give number of items in an id list
/// \param[in] list handle of an id list
size_t asapo_id_list_get_size(const AsapoIdListHandle list) {
    return list->handle->size();
}
//! give one items from an id list
/// \param[in] list handle of an id list
/// \param[in] index index of the item to return, start at 0
uint64_t asapo_id_list_get_item(const AsapoIdListHandle list,
                                size_t index) {
    return list->handle->at(index);
}
//! wraps asapo::Consumer::ForceNoRdma()
/// \copydoc asapo::Consumer::ForceNoRdma()
/// \param[in] consumer the handle of the consumer concerned
void asapo_consumer_force_no_rdma(AsapoConsumerHandle consumer);
//! wraps asapo::Consumer::CurrentConnectionType()
/// \copydoc asapo::Consumer::CurrentConnectionType()
/// \param[in] consumer the handle of the consumer concerned
enum AsapoNetworkConnectionType asapo_consumer_current_connection_type(AsapoConsumerHandle consumer);


//! get list of streams, wraps asapo::Consumer::GetStreamList()
/*!
  \param[in] consumer the consumer that is acted upon
  \param[in] from whatever
  \param[in] filter select the kind of stream
  \param[out] error will contain a pointer to an asapoError if a problem occured, NULL else.
  \return object that contains the stream infos
  \sa asapoStreamInfosGetInfo() asapo_stream_infos_get_size() asapo_delete_stream_infos()
*/
AsapoStreamInfosHandle asapo_consumer_get_stream_list(AsapoConsumerHandle consumer,
                                                      const char *from,
                                                      enum AsapoStreamFilter filter,
                                                      AsapoErrorHandle *error) {
    asapo::Error err;
    auto info = new asapo::StreamInfos(consumer->handle->GetStreamList(from,
                                                                       static_cast<asapo::StreamFilter>(filter),
                                                                       &err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<asapo::StreamInfos>{info};
}

//! get one stream info from a stream infos handle
/// \param[in] infos handle for stream infos
/// \param[in] index index od info to get, starts at 0
/// \return handle to stream info
AsapoStreamInfoHandle asapo_stream_infos_get_item(const AsapoStreamInfosHandle infos,
                                                  size_t index) {
    return new AsapoHandlerHolder<asapo::StreamInfo>{&(infos->handle->at(index)), false};
}

//! get size (number of elements) of a stream infos handle
/// \param[in] infos handle for stream infos
/// \return number of elements in the handle
size_t asapo_stream_infos_get_size(const AsapoStreamInfosHandle infos) {
    return infos->handle->size();
}
//! clean up streamInfos
/// frees the resources occupied by infos, sets *infos to NULL

void asapo_delete_stream_infos(AsapoStreamInfosHandle *infos) {
    delete *infos;
    *infos = nullptr;
}

//! wraps asapo::Consumer::DeleteStream()
/// \copydoc asapo::Consumer::DeleteStream()
/// \param[in] consumer the consumer that is acted upon
/// \param[in] delete_meta the delete_meta part of the asapo::DeleteStreamOptions
/// \param[in] error_on_not_exist the error_on_not_exist part of the asapo::DeleteStreamOptions
AsapoErrorHandle asapo_consumer_delete_stream(AsapoConsumerHandle consumer,
                                              const char *stream,
                                              AsapoBool delete_meta,
                                              AsapoBool error_on_not_exist) {
    asapo::DeleteStreamOptions opt(delete_meta, error_on_not_exist);
    auto err = consumer->handle->DeleteStream(stream, opt);
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}
//! wraps asapo::Consumer::GetCurrentSize()
/// \copydoc asapo::Consumer::GetCurrentSize()
/// \param[in] consumer the consumer that is acted upon
uint64_t asapo_consumer_get_current_size(AsapoConsumerHandle consumer,
                                         const char *stream,
                                         AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = consumer->handle->GetCurrentSize(stream, &err);
    process_error(error, std::move(err));
    return retval;
}
//! wraps asapo::Consumer::GetCurrentDatasetCount()
/// \copydoc asapo::Copydoc::GetCurrentDatasetCount()
/// \param[in] consumer the consumer that is acted upon
uint64_t asapo_consumer_get_current_dataset_count(AsapoConsumerHandle consumer,
                                                  const char *stream,
                                                  AsapoBool include_incomplete,
                                                  AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = consumer->handle->GetCurrentDatasetCount(stream, include_incomplete, &err);
    process_error(error, std::move(err));
    return retval;
}

//! wraps asapo::Consumer::GetBeamtimeMeta()
/// \copydoc asapo::Consumer::GetBeamtimeMeta()
/// \param[in] consumer the consumer that is acted upon
/// the returned string must be freed after use with asapo_delete_string()
AsapoStringHandle asapo_consumer_get_beamtime_meta(AsapoConsumerHandle consumer,
                                                   AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = new std::string(consumer->handle->GetBeamtimeMeta(&err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<std::string>{retval};
}

//! wraps asapo::Consumer::RetrieveData()
/// \copydoc asapo::Consumer::RetrieveData()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
AsapoErrorHandle asapo_consumer_retrieve_data(AsapoConsumerHandle consumer,
                                              AsapoMessageMetaHandle info,
                                              AsapoMessageDataHandle *data) {
    asapo::MessageData d;
    auto err = consumer->handle->RetrieveData(info->handle.get(), data ? &d : nullptr);
    if (data) {
        if (*data == nullptr) {
            *data = new AsapoHandlerHolder<uint8_t[]>{};
        }
            (*data)->handle = std::move(d);
    }
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}

//! wraps asapo::Consumer::GetNextDataset()
/// \copydoc asapo::Consumer::GetNextDataset()
/// \param[in] consumer the consumer that is acted upon
/// the returned data set must be freed with asapo_delete_data_set() after use.
AsapoDataSetHandle asapo_consumer_get_next_data_set(AsapoConsumerHandle consumer,
                                                    AsapoStringHandle group_id,
                                                    uint64_t min_size,
                                                    const char *stream,
                                                    AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = new asapo::DataSet(consumer->handle->GetNextDataset(*group_id->handle, min_size, stream, &err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<asapo::DataSet>{retval};
}

//! wraps asapo::Consumer::GetLastDataset()
/// \copydoc asapo::Consumer::GetLastDataset()
/// \param[in] consumer the consumer that is acted upon
/// the returned data set must be freed with asapo_delete_data_set() after use.
AsapoDataSetHandle asapo_consumer_get_last_data_set(AsapoConsumerHandle consumer,
                                                    uint64_t min_size,
                                                    const char *stream,
                                                    AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = new asapo::DataSet(consumer->handle->GetLastDataset(min_size, stream, &err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<asapo::DataSet>{retval};
}

//! wraps asapo::Consumer::GetLastAcknowledgedMessage()
/// \copydoc asapo::Consumer::GetLastAcknowledgedMessage()
/// \param[in] consumer the consumer that is acted upon
uint64_t asapo_consumer_get_last_acknowledged_message(AsapoConsumerHandle consumer,
                                                      AsapoStringHandle group_id,
                                                      const char *stream,
                                                      AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = consumer->handle->GetLastAcknowledgedMessage(*group_id->handle, stream, &err);
    process_error(error, std::move(err));
    return retval;
}

//! wraps asapo::Consumer::GetDatasetById()
/// \copydoc asapo::Consumer::GetDatasetById()
/// \param[in] consumer the consumer that is acted upon
/// the returned data set must be freed with asapo_delete_data_set() after use.
AsapoDataSetHandle asapo_consumer_get_data_set_by_id(AsapoConsumerHandle consumer,
                                                     uint64_t id,
                                                     uint64_t min_size,
                                                     const char *stream,
                                                     AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = new asapo::DataSet(consumer->handle->GetDatasetById(id, min_size, stream, &err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<asapo::DataSet>{retval};
}

//! wraps aspao::Consumer::GetById()
/// \copydoc aspao::Consumer::GetById()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
AsapoErrorHandle asapo_consumer_get_by_id(AsapoConsumerHandle consumer,
                                          uint64_t id,
                                          AsapoMessageMetaHandle *info,
                                          AsapoMessageDataHandle *data,
                                          const char *stream) {
    dataGetterStart;
    auto err = consumer->handle->GetById(id, fi, data ? &d : nullptr, stream);
    dataGetterStop;

    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}

//! wraps asapo::Consumer::GetLast()
/// \copydoc asapo::Consumer::GetLast()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
AsapoErrorHandle asapo_consumer_get_last(AsapoConsumerHandle consumer,
                                         AsapoMessageMetaHandle *info,
                                         AsapoMessageDataHandle *data,
                                         const char *stream) {
    dataGetterStart;
    auto err = consumer->handle->GetLast(fi, data ? &d : nullptr, stream);
    dataGetterStop;
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}

//! wraps asapo::Consumer::GetNext()
/// \copydoc asapo::Consumer::GetNext()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_delete_message_data()
AsapoErrorHandle asapo_consumer_get_next(AsapoConsumerHandle consumer,
                                         AsapoStringHandle group_id,
                                         AsapoMessageMetaHandle *info,
                                         AsapoMessageDataHandle *data,
                                         const char *stream) {
    dataGetterStart;
    auto err = consumer->handle->GetNext(*group_id->handle, fi, data ? &d : nullptr, stream);
    dataGetterStop;
    return new AsapoHandlerHolder<asapo::ErrorInterface>{err.release()};
}

//! wraps asapo::Consumer::QueryMessages()
/// \copydoc  asapo::Consumer::QueryMessages()
/// \param[in] consumer the consumer that is acted upon
/// the returned list must be freed with asapo_delete_message_metas() after use.
AsapoMessageMetasHandle asapo_consumer_query_messages(AsapoConsumerHandle consumer,
                                                      const char *query,
                                                      const char *stream,
                                                      AsapoErrorHandle *error) {
    asapo::Error err;
    auto retval = new asapo::MessageMetas(consumer->handle->QueryMessages(query, stream, &err));
    process_error(error, std::move(err));
    return new AsapoHandlerHolder<asapo::MessageMetas>{retval};
}

//! wraps aspao::Consumer::SetResendNacs()
/// \copydoc aspao::Consumer::SetResendNacs()
/// \param[in] consumer the consumer that is acted upon
void asapo_consumer_set_resend_nacs(AsapoConsumerHandle consumer,
                                    AsapoBool resend,
                                    uint64_t delay_ms,
                                    uint64_t resend_attempts) {
    consumer->handle->SetResendNacs(resend, delay_ms, resend_attempts);
}

//! clean up message data
/// \param[in] data the handle of the data
/// frees the resources occupied by data, sets *data to NULL
void asapo_delete_message_data(AsapoMessageDataHandle *data) {
    delete *data;
    *data = nullptr;
}

//! give acess to data
/// \param[in] data the handle of the data
/// \return const char pointer to the data blob, valid until deletion or reuse of data
const char *asapo_message_data_get_as_chars(const AsapoMessageDataHandle data) {
    return reinterpret_cast<const char *>(data->handle.get());
}

//! wraps asapo::SourceCredentials::SourceCredentials()
/// \copydoc asapo::SourceCredentials::SourceCredentials()
AsapoSourceCredentialsHandle asapo_create_source_credentials(enum AsapoSourceType type,
                                                             const char *beamtime,
                                                             const char *beamline,
                                                             const char *data_source,
                                                             const char *token) {
    auto retval = new asapo::SourceCredentials(static_cast<asapo::SourceType>(type),
                                               beamtime, beamline,
                                               data_source, token);
    return new AsapoHandlerHolder<asapo::SourceCredentials>{retval};
}

//! clean up sourceCredentials
///  frees the resources occupied by cred, sets *cred to NULL
void asapo_delete_source_credentials(AsapoSourceCredentialsHandle *cred) {
    delete *cred;
    cred = nullptr;
}

//! clean up asapoMessageMeta object
/// frees the resources occupied by meta, sets *meta to NULL
void asapo_delete_message_meta(AsapoMessageMetaHandle *meta) {
    delete *meta;
    *meta = nullptr;
}

//! get name from the metadata object
/// \param[in] md handle of the metadata object
/// \return pointer to the name string, valid until md is reused or deleted only!
const char *asapo_message_meta_get_name(const AsapoMessageMetaHandle md) {
    return md->handle->name.c_str();
}

//! get timestamp of the metadata object
/// \param[in] md handle of the metadata object
/// \param[out] stamp the timestamp as timespec
/// \sa asapo::MessageMeta
void asapo_message_meta_get_timestamp(const AsapoMessageMetaHandle md,
                                      struct timespec *stamp) {
    time_point_to_time_spec(md->handle->timestamp, stamp);
}

//! get size from the metadata object
/// \param[in] md handle of the metadata object
/// \return size of the associated data blob
/// \sa asapo::MessageMeta
uint64_t asapo_message_meta_get_size(const AsapoMessageMetaHandle md) {
    return md->handle->size;
}
//! get id from the metadata object
/// \param[in] md handle of the metadata object
/// \return id of the associated data blob
/// \sa asapo::MessageMeta
uint64_t asapo_message_meta_get_id(const AsapoMessageMetaHandle md) {
    return md->handle->id;
}
//! get source from the metadata object
/// \param[in] md handle of the metadata object
/// \return pointer to the source string, valid until md is reused or deleted only!
/// \sa asapo::MessageMeta
const char *asapo_message_meta_get_source(const AsapoMessageMetaHandle md) {
    return md->handle->source.c_str();
}
//! get metadata? from the metadata object
/// \param[in] md handle of the metadata object
/// \return pointer to the metadata string, valid until md is reused or deleted only!
/// \sa asapo::MessageMeta
const char *asapo_message_meta_get_metadata(const AsapoMessageMetaHandle md) {
    return md->handle->metadata.c_str();
}
//! get buffer id from the metadata object
/// \param[in] md handle of the metadata object
/// \return buffer id
/// \sa asapo::MessageMeta
uint64_t asapo_message_meta_get_buf_id(const AsapoMessageMetaHandle md) {
    return md->handle->buf_id;
}
//! get dataset substream id from the metadata object
/// \param[in] md handle of the metadata object
/// \return dataset substream id
/// \sa asapo::MessageMeta
uint64_t asapo_message_meta_get_dataset_substream(const AsapoMessageMetaHandle md) {
    return md->handle->dataset_substream;
}

//! get last id from the stream info object
/// \param[in] info handle of the stream info object
/// \return last id
/// \sa asapo::StreamInfo
uint64_t asapo_stream_info_get_last_id(const AsapoStreamInfoHandle info) {
    return info->handle->last_id;
}
//! get stream name from the stream info object
/// \param[in] info handle of the stream info object
/// \return  pointer to the name string, valid until asapoStreamInfos object is deleted
/// \sa asapo::StreamInfo
const char *asapo_stream_info_get_name(const AsapoStreamInfoHandle info) {
    return info->handle->name.c_str();
}
//! get finished state from the stream info object
/// \param[in] info handle of the stream info object
/// \return finised state, 0 = false
/// \sa asapo::StreamInfo
AsapoBool asapo_stream_info_get_ffinished(const AsapoStreamInfoHandle info) {
    return info->handle->finished;
}
//! get next stream name? from the stream info object
/// \param[in] info handle of the stream info object
/// \return  pointer to the name string, valid until asapoStreamInfos object is deleted
/// \sa asapo::StreamInfo
const char *asapo_stream_info_get_next_stream(const AsapoStreamInfoHandle info) {
    return info->handle->next_stream.c_str();
}
//! get creation time from the stream info object
/// \param[in] info handle of the stream info object
/// \param[out] stamp creation timestamp as timespec
/// \sa asapo::StreamInfo
void asapo_stream_info_get_timestamp_created(const AsapoStreamInfoHandle info,
                                             struct timespec *stamp) {
    time_point_to_time_spec(info->handle->timestamp_created, stamp);
}
//! get time of last entry from the stream info object
/// \param[in] info handle of the stream info object
/// \param[out] stamp last entry timestamp as timespec
/// \sa asapo::StreamInfo
void asapo_stream_info_get_timestamp_last_entry(const AsapoStreamInfoHandle info,
                                                struct timespec *stamp) {
    time_point_to_time_spec(info->handle->timestamp_lastentry, stamp);
}

//! clean up AsapoDataSet object
/// frees the resources occupied by set, sets *set to NULL
void asapo_delete_data_set(AsapoDataSetHandle *set) {
    delete *set;
    *set = nullptr;
}
//! get id from data set object
/// \param[in] set handle of the data set object
/// \return id of the data set
/// \sa asapo::DataSet
uint64_t asapo_data_set_get_id(const AsapoDataSetHandle set) {
    return set->handle->id;
}
//! get ecpected size from data set object
/// \param[in] set handle of the data set object
/// \return expected size of the data set
/// \sa asapo::DataSet
uint64_t asapo_data_set_get_expected_size(const AsapoDataSetHandle set) {
    return set->handle->expected_size;
}
//! get number of message meta objects from data set object
/// \param[in] set handle of the data set object
/// \return number of message meta objects of the data set
/// \sa asapo::DataSet
size_t asapo_data_set_get_size(const AsapoDataSetHandle set) {
    return set->handle->content.size();
}
//! get one message meta object handle from data set object
/// \param[in] set handle of the data set object
/// \param[in] index of the message meta object wanted
/// \return handle of the meta data object, do not delete!
/// \sa asapo::DataSet
AsapoMessageMetaHandle asapo_data_set_get_item(const AsapoDataSetHandle set,
                                               size_t index) {
    return new AsapoHandlerHolder<asapo::MessageMeta>{&(set->handle->content.at(index)), false};
}

//! clean up message metas object
/// frees the resources occupied by metas, sets *metas to NULL
void asapo_delete_message_metas(AsapoMessageMetasHandle *metas) {
    delete *metas;
    *metas = nullptr;
}
//! get number of message meta objects from metas object
/// \param[in] metas handle of the metas object
/// \return number of message meta objects of the data set
/// \sa asapo::MessageMetas
size_t asapo_message_metas_get_size(const AsapoMessageMetasHandle metas) {
    return metas->handle->size();
}
//! get one message meta object handle from metas object
/// \param[in] metas handle of the metas object
/// \param[in] index of the message meta object wanted
/// \return handle of the meta data object, do not delete!
/// \sa asapo::MessageMetas
AsapoMessageMetaHandle asapo_message_metas_get_item(const AsapoMessageMetasHandle metas,
                                                    size_t index) {
    return new AsapoHandlerHolder<asapo::MessageMeta>{&(metas->handle->at(index)), false};
}

void asapo_free_handle(void **handle) {
    auto a_handle = static_cast<AsapoHandle *>(*handle);
    delete a_handle;
    *handle = nullptr;
}

void* asapo_init_handle() {
    return NULL;
}

}
