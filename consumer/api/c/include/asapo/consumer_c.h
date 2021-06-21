#ifndef __CONSUMER_C_H__
#define __CONSUMER_C_H__

#ifndef __CONSUMER_C_INTERFACE_IMPLEMENTATION__
typedef int AsapoBool;
typedef void* AsapoConsumer;
typedef void* AsapoSourceCredentials;
typedef void* AsapoError;
typedef void* AsapoMessageMeta;
typedef void* AsapoMessageMetas;
typedef void* AsapoMessageData;
typedef void* AsapoString;
typedef void* AsapoStreamInfo;
typedef void* AsapoStreamInfos;
typedef void* AsapoIdList;
typedef void* AsapoDataSet;
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
void asapo_error_explain(const AsapoError error, char* buf, size_t max_size);
enum AsapoConsumerErrorType asapo_error_get_type(const AsapoError error);
void asapo_clear_error(AsapoError* error);

AsapoConsumer asapo_create_consumer(const char* server_name,
                                    const char* source_path,
                                    AsapoBool has_filesysytem,
                                    AsapoSourceCredentials source,
                                    AsapoError* error);
void asapo_delete_consumer(AsapoConsumer* consumer);
AsapoString asapo_consumer_generate_new_group_id(AsapoConsumer consumer, AsapoError* err);
AsapoString asapo_create_string(const char* content);
void asapo_string_append(AsapoString str, const char* content);
const char* asapo_string_c_str(const AsapoString str);
size_t asapo_string_size(const AsapoString str);
void asapo_delete_string(AsapoString* str);

void asapo_consumer_set_timeout(AsapoConsumer consumer, uint64_t timeout_ms);
AsapoError asapo_consumer_reset_last_read_marker(AsapoConsumer consumer,
                                                 const AsapoString group_id,
                                                 const char* stream);
AsapoError asapo_consumer_set_last_read_marker(AsapoConsumer consumer,
                                               const AsapoString group_id,
                                               uint64_t value,
                                               const char* stream);
AsapoError asapo_consumer_acknowledge(AsapoConsumer consumer,
                                      const AsapoString group_id,
                                      uint64_t id,
                                      const char* stream);
AsapoError asapo_consumer_negative_acknowledge(AsapoConsumer consumer,
                                               const AsapoString group_id,
                                               uint64_t id,
                                               uint64_t delay_ms,
                                               const char* stream);
AsapoIdList asapo_consumer_get_unacknowledged_messages(AsapoConsumer consumer,
        AsapoString group_id,
        uint64_t from_id,
        uint64_t to_id,
        const char* stream,
        AsapoError* error);
void asapo_delete_id_list(AsapoIdList* list);
size_t asapo_id_list_get_size(const AsapoIdList list);
uint64_t asapo_id_list_get_item(const AsapoIdList list,
                                size_t index);

uint64_t asapo_consumer_get_last_acknowledged_message(AsapoConsumer consumer,
        AsapoString group_id,
        const char* stream,
        AsapoError* error);

void asapo_consumer_force_no_rdma(AsapoConsumer consumer);
enum AsapoNetworkConnectionType asapo_consumer_current_connection_type(AsapoConsumer consumer);


AsapoStreamInfos asapo_consumer_get_stream_list(AsapoConsumer consumer,
                                                const char* from,
                                                enum AsapoStreamFilter filter,
                                                AsapoError* error);
const AsapoStreamInfo asapo_stream_infos_get_item(const AsapoStreamInfos infos,
                                                  size_t index);
size_t asapo_stream_infos_get_size(const AsapoStreamInfos infos);
void asapo_delete_stream_infos(AsapoStreamInfos* infos);

AsapoError asapo_consumer_delete_stream(AsapoConsumer consumer,
                                        const char* stream,
                                        AsapoBool delete_meta,
                                        AsapoBool error_on_not_exist);
uint64_t asapo_consumer_get_current_size(AsapoConsumer consumer,
                                         const char* stream,
                                         AsapoError* error);
uint64_t asapo_consumer_get_current_dataset_count(AsapoConsumer consumer,
                                                  const char* stream,
                                                  AsapoBool include_incomplete,
                                                  AsapoError* error);
AsapoString asapo_consumer_get_beamtime_meta(AsapoConsumer consumer,
                                             AsapoError* error);
AsapoError asapo_consumer_retrieve_data(AsapoConsumer consumer,
                                        AsapoMessageMeta* info,
                                        AsapoMessageData* data);
AsapoDataSet asapo_consumer_get_next_data_set(AsapoConsumer consumer,
                                              AsapoString group_id,
                                              uint64_t min_size,
                                              const char* stream,
                                              AsapoError* error);
AsapoDataSet asapo_consumer_get_last_data_set(AsapoConsumer consumer,
                                              uint64_t min_size,
                                              const char* stream,
                                              AsapoError* error);
AsapoDataSet asapo_consumer_get_data_set_by_id(AsapoConsumer consumer,
                                               uint64_t id,
                                               uint64_t min_size,
                                               const char* stream,
                                               AsapoError* error);
AsapoError asapo_consumer_get_by_id(AsapoConsumer consumer,
                                    uint64_t id,
                                    AsapoMessageMeta* info,
                                    AsapoMessageData* data,
                                    const char* stream);

AsapoError asapo_consumer_get_last(AsapoConsumer consumer,
                                   AsapoMessageMeta* info,
                                   AsapoMessageData* data,
                                   const char* stream);
AsapoError asapo_consumer_get_next(AsapoConsumer consumer,
                                   AsapoString group_id,
                                   AsapoMessageMeta* info,
                                   AsapoMessageData* data,
                                   const char* stream);
AsapoMessageMetas asapo_consumer_query_messages(AsapoConsumer consumer,
                                                const char* query,
                                                const char* stream,
                                                AsapoError* error);
void asapo_consumer_set_resend_nacs(AsapoConsumer consumer,
                                    AsapoBool resend,
                                    uint64_t delay_ms,
                                    uint64_t resend_attempts);

void asapo_delete_message_data(AsapoMessageData* data);
const char* asapo_message_data_get_as_chars(const AsapoMessageData data);
AsapoSourceCredentials asapo_create_source_credentials(enum AsapoSourceType type,
        const char* beamtime,
        const char* beamline,
        const char* data_source,
        const char* token);
void asapo_delete_source_credentials(AsapoSourceCredentials* cred);

void asapo_delete_message_meta(AsapoMessageMeta* meta);

const char* asapo_message_meta_get_name(const AsapoMessageMeta md);
void asapo_message_meta_get_timestamp(const AsapoMessageMeta md,
                                      struct timespec* stamp);
uint64_t asapo_message_meta_get_size(const AsapoMessageMeta md);
uint64_t asapo_message_meta_get_id(const AsapoMessageMeta md);
const char* asapo_message_meta_get_source(const AsapoMessageMeta md);
const char* asapo_message_meta_get_metadata(const AsapoMessageMeta md);
uint64_t asapo_message_meta_get_buf_id(const AsapoMessageMeta md);
uint64_t asapo_message_meta_get_dataset_substream(const AsapoMessageMeta md);

uint64_t asapo_stream_info_get_last_id(const AsapoStreamInfo info);
const char* asapo_stream_info_get_name(const AsapoStreamInfo info);
AsapoBool asapo_stream_info_get_ffinished(const AsapoStreamInfo info);
const char* asapo_stream_info_get_next_stream(const AsapoStreamInfo info);
void asapo_stream_info_get_timestamp_created(const AsapoStreamInfo info,
                                             struct timespec* stamp);
void asapo_stream_info_get_timestamp_last_entry(const AsapoStreamInfo info,
                                                struct timespec* stamp);

void asapo_delete_data_set(AsapoDataSet* set);
uint64_t asapo_data_set_get_id(const AsapoDataSet set);
uint64_t asapo_data_set_get_expected_size(const AsapoDataSet set);
size_t asapo_data_set_get_size(const AsapoDataSet set);
const AsapoMessageMeta asapo_data_set_get_item(const AsapoDataSet set,
                                               size_t index);

void asapo_delete_message_metas(AsapoMessageMetas* metas);
size_t asapo_message_metas_get_size(const AsapoMessageMetas metas);
const AsapoMessageMeta asapo_message_metas_get_item(const AsapoMessageMetas metas,
        size_t index);
#endif
