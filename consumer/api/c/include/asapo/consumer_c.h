#ifndef __CONSUMER_C_H__
#define __CONSUMER_C_H__

#ifndef __CONSUMER_C_INTERFACE_IMPLEMENTATION__
typedef int AsapoBool;

typedef void* AsapoConsumerHandle;
typedef void* AsapoSourceCredentialsHandle;
typedef void* AsapoErrorHandle;
typedef void* AsapoMessageMetaHandle;
typedef void* AsapoMessageMetasHandle;
typedef void* AsapoMessageDataHandle;
typedef void* AsapoStringHandle;
typedef void* AsapoStreamInfoHandle;
typedef void* AsapoStreamInfosHandle;
typedef void* AsapoIdListHandle;
typedef void* AsapoDataSetHandle;

#include <time.h>
#include <stdint.h>
#endif

//! c version of asapo::ConsumerErrorType
enum AsapoConsumerErrorType {
    kNoData = 0,
    kEndOfStream,
    kStreamFinished,
    kUnavailableService,
    kInterruptedTransaction,
    kLocalIOError,
    kWrongInput,
    kPartialData,
    kUnsupportedClient,
    kUnknownError
};

//! c version of asapo::StreamFilter
enum AsapoStreamFilter {
    kAllStreams,
    kFinishedStreams,
    kUnfinishedStreams
};
//! c version of asapo::SourceType
enum AsapoSourceType {
    kProcessed,
    kRaw
};
//! c version of asapo::NetworkConnectionType
enum AsapoNetworkConnectionType {
    kUndefined,
    kAsapoTcp,
    kFabric
};
void asapo_error_explain(const AsapoErrorHandle error, char* buf, size_t max_size);
enum AsapoConsumerErrorType asapo_error_get_type(const AsapoErrorHandle error);
void asapo_clear_error(AsapoErrorHandle* error);

AsapoConsumerHandle asapo_create_consumer(const char* server_name,
                                          const char* source_path,
                                          AsapoBool has_filesysytem,
                                          AsapoSourceCredentialsHandle source,
                                          AsapoErrorHandle* error);

AsapoBool asapo_is_error(AsapoErrorHandle err);

AsapoStringHandle asapo_consumer_generate_new_group_id(AsapoConsumerHandle consumer, AsapoErrorHandle* err);
const char* asapo_string_c_str(const AsapoStringHandle str);
size_t asapo_string_size(const AsapoStringHandle str);

void asapo_consumer_set_timeout(AsapoConsumerHandle consumer, uint64_t timeout_ms);
int asapo_consumer_reset_last_read_marker(AsapoConsumerHandle consumer,
                                          const AsapoStringHandle group_id,
                                          const char* stream,
                                          AsapoErrorHandle* error);
int asapo_consumer_set_last_read_marker(AsapoConsumerHandle consumer,
                                        const AsapoStringHandle group_id,
                                        uint64_t value,
                                        const char* stream, AsapoErrorHandle* error);
int asapo_consumer_acknowledge(AsapoConsumerHandle consumer,
                               const AsapoStringHandle group_id,
                               uint64_t id,
                               const char* stream, AsapoErrorHandle* error);
int asapo_consumer_negative_acknowledge(AsapoConsumerHandle consumer,
                                        const AsapoStringHandle group_id,
                                        uint64_t id,
                                        uint64_t delay_ms,
                                        const char* stream, AsapoErrorHandle* error);
AsapoIdListHandle asapo_consumer_get_unacknowledged_messages(AsapoConsumerHandle consumer,
        AsapoStringHandle group_id,
        uint64_t from_id,
        uint64_t to_id,
        const char* stream,
        AsapoErrorHandle* error);
size_t asapo_id_list_get_size(const AsapoIdListHandle list);
uint64_t asapo_id_list_get_item(const AsapoIdListHandle list,
                                size_t index);

int64_t asapo_consumer_get_last_acknowledged_message(AsapoConsumerHandle consumer,
        AsapoStringHandle group_id,
        const char* stream,
        AsapoErrorHandle* error);

void asapo_consumer_force_no_rdma(AsapoConsumerHandle consumer);
enum AsapoNetworkConnectionType asapo_consumer_current_connection_type(AsapoConsumerHandle consumer);

AsapoStreamInfosHandle asapo_consumer_get_stream_list(AsapoConsumerHandle consumer,
        const char* from,
        enum AsapoStreamFilter filter,
        AsapoErrorHandle* error);
AsapoStreamInfoHandle asapo_stream_infos_get_item(const AsapoStreamInfosHandle infos,
                                                  size_t index);
size_t asapo_stream_infos_get_size(const AsapoStreamInfosHandle infos);

int asapo_consumer_delete_stream(AsapoConsumerHandle consumer,
                                 const char* stream,
                                 AsapoBool delete_meta,
                                 AsapoBool error_on_not_exist, AsapoErrorHandle* error);
int64_t asapo_consumer_get_current_size(AsapoConsumerHandle consumer,
                                        const char* stream,
                                        AsapoErrorHandle* error);
int64_t asapo_consumer_get_current_dataset_count(AsapoConsumerHandle consumer,
                                                 const char* stream,
                                                 AsapoBool include_incomplete,
                                                 AsapoErrorHandle* error);
AsapoStringHandle asapo_consumer_get_beamtime_meta(AsapoConsumerHandle consumer,
        AsapoErrorHandle* error);
int asapo_consumer_retrieve_data(AsapoConsumerHandle consumer,
                                 AsapoMessageMetaHandle info,
                                 AsapoMessageDataHandle* data, AsapoErrorHandle* error);
AsapoDataSetHandle asapo_consumer_get_next_dataset(AsapoConsumerHandle consumer,
        AsapoStringHandle group_id,
        uint64_t min_size,
        const char* stream,
        AsapoErrorHandle* error);
AsapoDataSetHandle asapo_consumer_get_last_dataset(AsapoConsumerHandle consumer,
        uint64_t min_size,
        const char* stream,
        AsapoErrorHandle* error);
AsapoDataSetHandle asapo_consumer_get_dataset_by_id(AsapoConsumerHandle consumer,
        uint64_t id,
        uint64_t min_size,
        const char* stream,
        AsapoErrorHandle* error);
int asapo_consumer_get_by_id(AsapoConsumerHandle consumer,
                             uint64_t id,
                             AsapoMessageMetaHandle* info,
                             AsapoMessageDataHandle* data,
                             const char* stream, AsapoErrorHandle* error);

int asapo_consumer_get_last(AsapoConsumerHandle consumer,
                            AsapoMessageMetaHandle* info,
                            AsapoMessageDataHandle* data,
                            const char* stream, AsapoErrorHandle* error);
int asapo_consumer_get_next(AsapoConsumerHandle consumer,
                            AsapoStringHandle group_id,
                            AsapoMessageMetaHandle* info,
                            AsapoMessageDataHandle* data,
                            const char* stream, AsapoErrorHandle* error);
AsapoMessageMetasHandle asapo_consumer_query_messages(AsapoConsumerHandle consumer,
        const char* query,
        const char* stream,
        AsapoErrorHandle* error);
void asapo_consumer_set_resend_nacs(AsapoConsumerHandle consumer,
                                    AsapoBool resend,
                                    uint64_t delay_ms,
                                    uint64_t resend_attempts);

const char* asapo_message_data_get_as_chars(const AsapoMessageDataHandle data);
AsapoSourceCredentialsHandle asapo_create_source_credentials(enum AsapoSourceType type,
        const char* beamtime,
        const char* beamline,
        const char* data_source,
        const char* token);

const char* asapo_message_meta_get_name(const AsapoMessageMetaHandle md);
void asapo_message_meta_get_timestamp(const AsapoMessageMetaHandle md,
                                      struct timespec* stamp);
uint64_t asapo_message_meta_get_size(const AsapoMessageMetaHandle md);
uint64_t asapo_message_meta_get_id(const AsapoMessageMetaHandle md);
const char* asapo_message_meta_get_source(const AsapoMessageMetaHandle md);
const char* asapo_message_meta_get_metadata(const AsapoMessageMetaHandle md);
uint64_t asapo_message_meta_get_buf_id(const AsapoMessageMetaHandle md);
uint64_t asapo_message_meta_get_dataset_substream(const AsapoMessageMetaHandle md);

uint64_t asapo_stream_info_get_last_id(const AsapoStreamInfoHandle info);
const char* asapo_stream_info_get_name(const AsapoStreamInfoHandle info);
AsapoBool asapo_stream_info_get_ffinished(const AsapoStreamInfoHandle info);
const char* asapo_stream_info_get_next_stream(const AsapoStreamInfoHandle info);
void asapo_stream_info_get_timestamp_created(const AsapoStreamInfoHandle info,
                                             struct timespec* stamp);
void asapo_stream_info_get_timestamp_last_entry(const AsapoStreamInfoHandle info,
                                                struct timespec* stamp);

uint64_t asapo_dataset_get_id(const AsapoDataSetHandle set);
uint64_t asapo_dataset_get_expected_size(const AsapoDataSetHandle set);
size_t asapo_dataset_get_size(const AsapoDataSetHandle set);
AsapoMessageMetaHandle asapo_dataset_get_item(const AsapoDataSetHandle set,
                                              size_t index);

size_t asapo_message_metas_get_size(const AsapoMessageMetasHandle metas);
AsapoMessageMetaHandle asapo_message_metas_get_item(const AsapoMessageMetasHandle metas,
        size_t index);

void asapo_free_handle(void** handle);
void* asapo_new_handle();

#endif
