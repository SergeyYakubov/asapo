#define __CONSUMER_C_INTERFACE_IMPLEMENTATION__
#include "asapo/asapo_consumer.h"
#include "asapo/common/internal/asapo_common_c_glue.h"

//! boolean type
typedef int AsapoBool;


//! handle for an asapo consumer
/// created by asapo_create_consumer()
/// free after use with asapo_free_handle()
/// all operations are done with asapo_consumer_xxx() functions
/// \sa asapo::Consumer
typedef AsapoHandlerHolder<asapo::Consumer>* AsapoConsumerHandle;

//! handle for metadata of a message
/// create with asapo_new_handle()
/// free after use with asapo_free_handle()
/// A set of getters asapoMessageMetaGetXxx() are defined
/// \sa asapo::MessageMeta
typedef AsapoHandlerHolder<asapo::MessageMeta>* AsapoMessageMetaHandle;

//! handle for set of metadata of messages
/// create with asapo_new_handle()
/// free after use with asapo_free_handle()
/// \sa asapo::MessageMetas
typedef AsapoHandlerHolder<asapo::MessageMetas>* AsapoMessageMetasHandle;



//! handle for message id lists
/// touch only with proper functions and use asapo_free_handle() to delete,
/// created by asapo_consumer_get_unacknowledged_messages()
/// \sa asapo::IdList asapo_id_list_get_size() asapo_id_list_get_item()
typedef AsapoHandlerHolder<asapo::IdList>* AsapoIdListHandle;

//! handle for data sets
/// touch only with proper functions and use asapo_free_handle() to delete
typedef AsapoHandlerHolder<asapo::DataSet>* AsapoDataSetHandle;

//! handle for partial error payload
/// create with asapo_new_handle()
/// free after use with asapo_free_handle()
/// A set of getters asapo_partial_error_get_xx() are defined
typedef AsapoHandlerHolder<asapo::PartialErrorData>* AsapoPartialErrorDataHandle;

//! handle for consumer error payload
/// create with asapo_new_handle()
/// free after use with asapo_free_handle()
/// A set of getters asapo_consumer_error_get_xx() are defined
typedef AsapoHandlerHolder<asapo::ConsumerErrorData>* AsapoConsumerErrorDataHandle;

#include <algorithm>
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
    static_assert(kNoData == asapo::ConsumerErrorType::kNoData&&
                  kEndOfStream == asapo::ConsumerErrorType::kEndOfStream&&
                  kStreamFinished == asapo::ConsumerErrorType::kStreamFinished&&
                  kUnavailableService == asapo::ConsumerErrorType::kUnavailableService&&
                  kInterruptedTransaction == asapo::ConsumerErrorType::kInterruptedTransaction&&
                  kLocalIOError == asapo::ConsumerErrorType::kLocalIOError&&
                  kWrongInput == asapo::ConsumerErrorType::kWrongInput&&
                  kPartialData == asapo::ConsumerErrorType::kPartialData&&
                  kUnsupportedClient == asapo::ConsumerErrorType::kUnsupportedClient,
                  "incompatible bit reps between c++ and c for asapo::ConsumerErrorType");
    static_assert(kAllStreams == asapo::StreamFilter::kAllStreams&&
                  kFinishedStreams == asapo::StreamFilter::kFinishedStreams&&
                  kUnfinishedStreams == asapo::StreamFilter::kUnfinishedStreams,
                  "incompatible bit reps between c++ and c for asapo::StreamFilter");
    static_assert(kUndefined == asapo::NetworkConnectionType::kUndefined&&
                  kAsapoTcp == asapo::NetworkConnectionType::kAsapoTcp&&
                  kFabric == asapo::NetworkConnectionType::kFabric,
                  "incompatible bit reps between c++ and c for asapo::NetworkConnectionType");

    enum AsapoConsumerErrorType asapo_error_get_type(const AsapoErrorHandle error) {
        auto consumer_err =
            dynamic_cast<const asapo::ServiceError<asapo::ConsumerErrorType> *>(error->handle.get());
        if (consumer_err != nullptr) {
            return static_cast<AsapoConsumerErrorType>(consumer_err->GetServiceErrorType());
        } else {
            return kUnknownError;
        }
    }


//! creata a consumer
/// \copydoc asapo::ConsumerFactory::CreateConsumer
/// \param[out] error NULL or pointer to error handle to be set
/// return handle to the created consumer
    AsapoConsumerHandle asapo_create_consumer(const char* server_name,
                                              const char* source_path,
                                              AsapoBool has_filesysytem,
                                              AsapoSourceCredentialsHandle source,
                                              AsapoErrorHandle* error) {

        asapo::Error err;
        auto c = asapo::ConsumerFactory::CreateConsumer(server_name,
                                                        source_path,
                                                        has_filesysytem,
                                                        *(source->handle),
                                                        &err);
        return handle_or_null_t(c, error, std::move(err));
    }

//! wraps asapo::Consumer::GenerateNewGroupId()
/// \copydoc asapo::Consumer::GenerateNewGroupId()
/// \param[in] consumer the handle of the consumer concerned
/// \param[out] error NULL or pointer to error handle to be set
/// \return AsapoStringHandle which is NULL in case of error
    AsapoStringHandle asapo_consumer_generate_new_group_id(AsapoConsumerHandle consumer,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto result = consumer->handle->GenerateNewGroupId(&err);
        return handle_or_null_t(result, error, std::move(err));
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
/// \param[out] error NULL or pointer to error handle to be set
/// \return 0 if completed successfully, -1 in case of error.
    int asapo_consumer_reset_last_read_marker(AsapoConsumerHandle consumer,
                                              const AsapoStringHandle group_id,
                                              const char* stream,
                                              AsapoErrorHandle* error) {
        auto err = consumer->handle->ResetLastReadMarker(*group_id->handle, stream);
        return process_error(error, std::move(err));
    }

//! wraps asapo::Consumer::SetLastReadMarker()
/// \copydoc asapo::Consumer::SetLastReadMarker()
/// \param[in] consumer the handle of the consumer concerned
/// \param[out] error NULL or pointer to error handle to be set
/// \return 0 if completed successfully, -1 in case of error.
    int asapo_consumer_set_last_read_marker(AsapoConsumerHandle consumer,
                                            const AsapoStringHandle group_id,
                                            uint64_t value,
                                            const char* stream,
                                            AsapoErrorHandle* error) {
        auto err = consumer->handle->SetLastReadMarker(*group_id->handle, value, stream);
        return process_error(error, std::move(err));
    }
//! wraps asapo::Consumer::Acknowledge()
/// \copydoc asapo::Consumer::Acknowledge()
/// \param[in] consumer the handle of the consumer concerned
/// \param[out] error NULL or pointer to error handle to be set
/// \return 0 if completed successfully, -1 in case of error.
    int asapo_consumer_acknowledge(AsapoConsumerHandle consumer,
                                   const AsapoStringHandle group_id,
                                   uint64_t id,
                                   const char* stream,
                                   AsapoErrorHandle* error) {
        auto err = consumer->handle->Acknowledge(*group_id->handle, id, stream);
        return process_error(error, std::move(err));
    }
//! wraps asapo::Consumer::NegativeAcknowledge()
/// \copydoc asapo::Consumer::NegativeAcknowledge()
/// \param[in] consumer the handle of the consumer concerned
/// \param[out] error NULL or pointer to error handle to be set
/// \return 0 if completed successfully, -1 in case of error.
    int asapo_consumer_negative_acknowledge(AsapoConsumerHandle consumer,
                                            const AsapoStringHandle group_id,
                                            uint64_t id,
                                            uint64_t delay_ms,
                                            const char* stream,
                                            AsapoErrorHandle* error) {
        auto err = consumer->handle->NegativeAcknowledge(*group_id->handle, id, delay_ms, stream);
        return process_error(error, std::move(err));
    }

//! wraps asapo::Consumer::GetUnacknowledgedMessages()
/// \copydoc asapo::Consumer::GetUnacknowledgedMessages()
/// \param[in] consumer the handle of the consumer concerned
/// \param[out] error NULL or pointer to error handle to be set
/// \return AsapoIdListHandle which is NULL in case of error
    AsapoIdListHandle asapo_consumer_get_unacknowledged_messages(AsapoConsumerHandle consumer,
            AsapoStringHandle group_id,
            uint64_t from_id,
            uint64_t to_id,
            const char* stream,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto list = new asapo::IdList(consumer->handle->GetUnacknowledgedMessages(*group_id->handle,
                                      from_id, to_id,
                                      stream,
                                      &err));
        return handle_or_null_t(list, error, std::move(err));
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
    enum AsapoNetworkConnectionType asapo_consumer_current_connection_type(AsapoConsumerHandle consumer) {
        return static_cast<AsapoNetworkConnectionType>(consumer->handle->CurrentConnectionType());
    }


//! get list of streams, wraps asapo::Consumer::GetStreamList()
    /*!
      \param[in] consumer the consumer that is acted upon
      \param[in] from whatever
      \param[in] filter select the kind of stream
      \param[out] error will contain a pointer to an AsapoErrorHandle if a problem occured, NULL else.
      \return object that contains the stream infos or NULL in case of error
      \sa asapoStreamInfosGetInfo() asapo_stream_infos_get_size() asapo_free_handle()
    */
    AsapoStreamInfosHandle asapo_consumer_get_stream_list(AsapoConsumerHandle consumer,
            const char* from,
            enum AsapoStreamFilter filter,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto info = new asapo::StreamInfos(consumer->handle->GetStreamList(from,
                                           static_cast<asapo::StreamFilter>(filter),
                                           &err));
        return handle_or_null_t(info, error, std::move(err));
    }


//! wraps asapo::Consumer::DeleteStream()
/// \copydoc asapo::Consumer::DeleteStream()
/// \param[in] consumer the consumer that is acted upon
/// \param[in] delete_meta the delete_meta part of the asapo::DeleteStreamOptions
/// \param[in] error_on_not_exist the error_on_not_exist part of the asapo::DeleteStreamOptions
/// \param[out] error NULL or pointer to error handle to be set
/// \return 0 if completed successfully, -1 in case of error.
    int asapo_consumer_delete_stream(AsapoConsumerHandle consumer,
                                     const char* stream,
                                     AsapoBool delete_meta,
                                     AsapoBool error_on_not_exist,
                                     AsapoErrorHandle* error) {
        asapo::DeleteStreamOptions opt(delete_meta, error_on_not_exist);
        auto err = consumer->handle->DeleteStream(stream, opt);
        return process_error(error, std::move(err));
    }
//! wraps asapo::Consumer::GetCurrentSize()
/// \copydoc asapo::Consumer::GetCurrentSize()
/// \param[in] consumer the consumer that is acted upon
/// \param[out] error NULL or pointer to error handle to be set
/// \return current stream size if completed successfully, -1 in case of error.
    int64_t asapo_consumer_get_current_size(AsapoConsumerHandle consumer,
                                            const char* stream,
                                            AsapoErrorHandle* error) {
        asapo::Error err;
        auto retval = consumer->handle->GetCurrentSize(stream, &err);
        process_error(error, std::move(err));
        return int64_t(retval);
    }
//! wraps asapo::Consumer::GetCurrentDatasetCount()
/// \copydoc asapo::Copydoc::GetCurrentDatasetCount()
/// \param[in] consumer the consumer that is acted upon
/// \param[out] error NULL or pointer to error handle to be set
/// \return dataset count if completed successfully, -1 in case of error.
    int64_t asapo_consumer_get_current_dataset_count(AsapoConsumerHandle consumer,
                                                     const char* stream,
                                                     AsapoBool include_incomplete,
                                                     AsapoErrorHandle* error) {
        asapo::Error err;
        auto retval = consumer->handle->GetCurrentDatasetCount(stream, include_incomplete, &err);
        if (process_error(error, std::move(err)) < 0) {
            return -1;
        }
        return int64_t(retval);
    }

//! wraps asapo::Consumer::GetBeamtimeMeta()
/// \copydoc asapo::Consumer::GetBeamtimeMeta()
/// \param[in] consumer the consumer that is acted upon
/// the returned string must be freed after use with asapo_free_handle()
    AsapoStringHandle asapo_consumer_get_beamtime_meta(AsapoConsumerHandle consumer,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto result = consumer->handle->GetBeamtimeMeta(&err);
        return handle_or_null_t(result, error, std::move(err));
    }

//! wraps asapo::Consumer::RetrieveData()
/// \copydoc asapo::Consumer::RetrieveData()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_free_handle()
    int asapo_consumer_retrieve_data(AsapoConsumerHandle consumer,
                                     AsapoMessageMetaHandle info,
                                     AsapoMessageDataHandle* data,
                                     AsapoErrorHandle* error) {
        asapo::MessageData d;
        auto err = consumer->handle->RetrieveData(info->handle.get(), data ? &d : nullptr);
        if (data) {
            if (*data == nullptr) {
                *data = new AsapoHandlerHolder<uint8_t[]> {};
            }
            (*data)->handle = std::move(d);
        }
        return process_error(error, std::move(err));
    }

//! wraps asapo::Consumer::GetNextDataset()
/// \copydoc asapo::Consumer::GetNextDataset()
/// \param[in] consumer the consumer that is acted upon
/// the returned data set must be freed with asapo_free_handle() after use.
    AsapoDataSetHandle asapo_consumer_get_next_dataset(AsapoConsumerHandle consumer,
            AsapoStringHandle group_id,
            uint64_t min_size,
            const char* stream,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto result = new asapo::DataSet(consumer->handle->GetNextDataset(*group_id->handle, min_size, stream, &err));
        return handle_or_null_t(result, error, std::move(err),
                                &asapo::ConsumerErrorTemplates::kPartialData);
    }

//! wraps asapo::Consumer::GetLastDataset()
/// \copydoc asapo::Consumer::GetLastDataset()
/// \param[in] consumer the consumer that is acted upon
/// the returned data set must be freed with asapo_free_handle() after use.
    AsapoDataSetHandle asapo_consumer_get_last_dataset(AsapoConsumerHandle consumer,
            uint64_t min_size,
            const char* stream,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto result = new asapo::DataSet(consumer->handle->GetLastDataset(min_size, stream, &err));
        return handle_or_null_t(result, error, std::move(err),
                                &asapo::ConsumerErrorTemplates::kPartialData);
    }
//! wraps asapo::Consumer::GetLastDataset()
/// \copydoc asapo::Consumer::GetLastDataset()
/// \param[in] consumer the consumer that is acted upon
/// the returned data set must be freed with asapo_free_handle() after use.
    AsapoDataSetHandle asapo_consumer_get_last_dataset_ingroup(AsapoConsumerHandle consumer,
            AsapoStringHandle group_id,
            uint64_t min_size,
            const char* stream,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto result = new asapo::DataSet(consumer->handle->GetLastDataset(*group_id->handle, min_size, stream, &err));
        return handle_or_null_t(result, error, std::move(err),
                                &asapo::ConsumerErrorTemplates::kPartialData);
    }


//! wraps asapo::Consumer::GetLastAcknowledgedMessage()
/// \copydoc asapo::Consumer::GetLastAcknowledgedMessage()
/// \param[in] consumer the consumer that is acted upon
/// \return index of the last acknowledged message, -1 in case of error.
    int64_t asapo_consumer_get_last_acknowledged_message(AsapoConsumerHandle consumer,
            AsapoStringHandle group_id,
            const char* stream,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto retval = consumer->handle->GetLastAcknowledgedMessage(*group_id->handle, stream, &err);
        if (process_error(error, std::move(err)) < 0) {
            return -1;
        }
        return static_cast<int64_t>(retval);
    }

//! wraps asapo::Consumer::GetDatasetById()
/// \copydoc asapo::Consumer::GetDatasetById()
/// \param[in] consumer the consumer that is acted upon
/// the returned data set must be freed with asapo_free_handle() after use.
    AsapoDataSetHandle asapo_consumer_get_dataset_by_id(AsapoConsumerHandle consumer,
            uint64_t id,
            uint64_t min_size,
            const char* stream,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto result = new asapo::DataSet(consumer->handle->GetDatasetById(id, min_size, stream, &err));
        return handle_or_null_t(result, error, std::move(err),
                                &asapo::ConsumerErrorTemplates::kPartialData);
    }

//! wraps aspao::Consumer::GetById()
/// \copydoc aspao::Consumer::GetById()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_free_handle()
    int asapo_consumer_get_by_id(AsapoConsumerHandle consumer,
                                 uint64_t id,
                                 AsapoMessageMetaHandle* info,
                                 AsapoMessageDataHandle* data,
                                 const char* stream,
                                 AsapoErrorHandle* error) {
        dataGetterStart
        auto err = consumer->handle->GetById(id, fi, data ? &d : nullptr, stream);
        dataGetterStop

        return process_error(error, std::move(err));
    }

//! wraps asapo::Consumer::GetLast()
/// \copydoc asapo::Consumer::GetLast()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_free_handle()
    int asapo_consumer_get_last(AsapoConsumerHandle consumer,
                                AsapoMessageMetaHandle* info,
                                AsapoMessageDataHandle* data,
                                const char* stream,
                                AsapoErrorHandle* error) {
        dataGetterStart
        auto err = consumer->handle->GetLast(fi, data ? &d : nullptr, stream);
        dataGetterStop

        return process_error(error, std::move(err));
    }

//! wraps asapo::Consumer::GetLast()
/// \copydoc asapo::Consumer::GetLast()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_free_handle()
    int asapo_consumer_get_last_ingroup(AsapoConsumerHandle consumer,
                                        AsapoStringHandle group_id,
                                        AsapoMessageMetaHandle* info,
                                        AsapoMessageDataHandle* data,
                                        const char* stream,
                                        AsapoErrorHandle* error) {
        dataGetterStart
        auto err = consumer->handle->GetLast(*group_id->handle, fi, data ? &d : nullptr, stream);
        dataGetterStop

        return process_error(error, std::move(err));
    }

//! wraps asapo::Consumer::GetNext()
/// \copydoc asapo::Consumer::GetNext()
/// \param[in] consumer the consumer that is acted upon
/// if data are retrieved (data != NULL) they must be freed with asapo_free_handle()
    int asapo_consumer_get_next(AsapoConsumerHandle consumer,
                                AsapoStringHandle group_id,
                                AsapoMessageMetaHandle* info,
                                AsapoMessageDataHandle* data,
                                const char* stream,
                                AsapoErrorHandle* error) {
        dataGetterStart

        auto err = consumer->handle->GetNext(*group_id->handle, fi, data ? &d : nullptr, stream);
        dataGetterStop

        return process_error(error, std::move(err));
    }

//! wraps asapo::Consumer::QueryMessages()
/// \copydoc  asapo::Consumer::QueryMessages()
/// \param[in] consumer the consumer that is acted upon
/// the returned list must be freed with asapo_free_handle() after use.
    AsapoMessageMetasHandle asapo_consumer_query_messages(AsapoConsumerHandle consumer,
            const char* query,
            const char* stream,
            AsapoErrorHandle* error) {
        asapo::Error err;
        auto result = new asapo::MessageMetas(consumer->handle->QueryMessages(query, stream, &err));
        return handle_or_null_t(result, error, std::move(err));
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


//! get name from the metadata object
/// \param[in] md handle of the metadata object
/// \return pointer to the name string, valid until md is reused or deleted only!
    const char* asapo_message_meta_get_name(const AsapoMessageMetaHandle md) {
        return md->handle->name.c_str();
    }

//! get timestamp of the metadata object
/// \param[in] md handle of the metadata object
/// \param[out] stamp the timestamp as timespec
/// \sa asapo::MessageMeta
    void asapo_message_meta_get_timestamp(const AsapoMessageMetaHandle md,
                                          struct timespec* stamp) {
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
    const char* asapo_message_meta_get_source(const AsapoMessageMetaHandle md) {
        return md->handle->source.c_str();
    }
//! get metadata? from the metadata object
/// \param[in] md handle of the metadata object
/// \return pointer to the metadata string, valid until md is reused or deleted only!
/// \sa asapo::MessageMeta
    const char* asapo_message_meta_get_metadata(const AsapoMessageMetaHandle md) {
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


//! get id from data set object
/// \param[in] set handle of the data set object
/// \return id of the data set
/// \sa asapo::DataSet
    uint64_t asapo_dataset_get_id(const AsapoDataSetHandle set) {
        return set->handle->id;
    }
//! get ecpected size from data set object
/// \param[in] set handle of the data set object
/// \return expected size of the data set
/// \sa asapo::DataSet
    uint64_t asapo_dataset_get_expected_size(const AsapoDataSetHandle set) {
        return set->handle->expected_size;
    }
//! get number of message meta objects from data set object
/// \param[in] set handle of the data set object
/// \return number of message meta objects of the data set
/// \sa asapo::DataSet
    size_t asapo_dataset_get_size(const AsapoDataSetHandle set) {
        return set->handle->content.size();
    }
//! get one message meta object handle from data set object
/// \param[in] set handle of the data set object
/// \param[in] index of the message meta object wanted
/// \return handle of the meta data object
/// \sa asapo::DataSet
    AsapoMessageMetaHandle asapo_dataset_get_item(const AsapoDataSetHandle set,
                                                  size_t index) {
        return new AsapoHandlerHolder<asapo::MessageMeta> {&(set->handle->content.at(index)), false};
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
/// \return handle of the meta data object
/// \sa asapo::MessageMetas
    AsapoMessageMetaHandle asapo_message_metas_get_item(const AsapoMessageMetasHandle metas,
            size_t index) {
        return new AsapoHandlerHolder<asapo::MessageMeta> {&(metas->handle->at(index)), false};
    }


//! get payload from partial error
/// \param[in] asapo error
/// \return handle to partial error data or NULL if error is wrong type
    AsapoPartialErrorDataHandle asapo_error_get_payload_from_partial_error(const AsapoErrorHandle error) {
        if (error == nullptr || error->handle == nullptr) {
            return nullptr;
        }
        auto payload = dynamic_cast<asapo::PartialErrorData*>(error->handle->GetCustomData());
        if (payload == nullptr) {
            return nullptr;
        }
        return new AsapoHandlerHolder<asapo::PartialErrorData> {payload, false};
    }


//! get id from the partial error object
/// \param[in] error_payload handle of the partial error data object
/// \sa asapo::PartialErrorData
    uint64_t asapo_partial_error_get_id(const AsapoPartialErrorDataHandle error_payload) {
        return error_payload->handle->id;
    }

//! get expected dataset size from the partial error object
/// \param[in] error_payload handle of the partial error data object
/// \sa asapo::PartialErrorData
    uint64_t asapo_partial_error_get_expected_size(const AsapoPartialErrorDataHandle error_payload) {
        return error_payload->handle->expected_size;
    }

//! get payload from consumer error
/// \param[in] asapo error
/// \return handle to partial error data or NULL if error is wrong type
    AsapoConsumerErrorDataHandle asapo_error_get_payload_from_consumer_error(const AsapoErrorHandle error) {
        if (error == nullptr || error->handle == nullptr) {
            return nullptr;
        }
        auto payload = dynamic_cast<asapo::ConsumerErrorData*>(error->handle->GetCustomData());
        if (payload == nullptr) {
            return nullptr;
        }
        return new AsapoHandlerHolder<asapo::ConsumerErrorData> {payload, false};
    }

//! get id from the consumer error data object
/// \param[in] error_payload handle of the consumer error data object
/// \sa asapo::ConsumerErrorData
    uint64_t asapo_consumer_error_get_id(const AsapoConsumerErrorDataHandle error_payload) {
        return error_payload->handle->id;
    }

//! get id_max from the consumer error data object
/// \param[in] error_payload handle of the consumer error data object
/// \sa asapo::ConsumerErrorData
    uint64_t asapo_consumer_error_get_id_max(const AsapoConsumerErrorDataHandle error_payload) {
        return error_payload->handle->id_max;
    }

//! get next_stream from the consumer error data object
/// \param[in] error_payload handle of the consumer error data object
/// \sa asapo::ConsumerErrorData
    const char* asapo_consumer_error_get_next_stream(const AsapoConsumerErrorDataHandle error_payload) {
        return error_payload->handle->next_stream.c_str();
    }



}
