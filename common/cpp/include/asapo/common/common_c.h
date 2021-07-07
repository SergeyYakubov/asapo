#ifndef __COMMON_C_H__
#define __COMMON_C_H__
#include <stddef.h>
#include <stdint.h>
#include <time.h>

const unsigned AsapoHandleSize = 24;
typedef int AsapoBool;
#ifndef __COMMON_C_INTERFACE_IMPLEMENTATION__
typedef void* AsapoSourceCredentialsHandle;
typedef void* AsapoErrorHandle;
typedef void* AsapoStringHandle;
typedef void* AsapoStreamInfoHandle;
typedef void* AsapoStreamInfosHandle;
typedef void* AsapoMessageDataHandle;
#endif

//! c version of asapo::SourceType
enum AsapoSourceType {
    kProcessed,
    kRaw
};


void asapo_free_handle(void** handle);
void* asapo_new_handle();


void asapo_error_explain(const AsapoErrorHandle error, char* buf, size_t max_size);
AsapoBool asapo_is_error(AsapoErrorHandle err);

AsapoStringHandle asapo_string_create(const char* str);
const char* asapo_string_c_str(const AsapoStringHandle str);
size_t asapo_string_size(const AsapoStringHandle str);

AsapoStreamInfoHandle asapo_stream_infos_get_item(const AsapoStreamInfosHandle infos,
                                                  size_t index);
size_t asapo_stream_infos_get_size(const AsapoStreamInfosHandle infos);

uint64_t asapo_stream_info_get_last_id(const AsapoStreamInfoHandle info);
const char* asapo_stream_info_get_name(const AsapoStreamInfoHandle info);
AsapoBool asapo_stream_info_get_ffinished(const AsapoStreamInfoHandle info);
const char* asapo_stream_info_get_next_stream(const AsapoStreamInfoHandle info);
void asapo_stream_info_get_timestamp_created(const AsapoStreamInfoHandle info,
                                             struct timespec* stamp);
void asapo_stream_info_get_timestamp_last_entry(const AsapoStreamInfoHandle info,
                                                struct timespec* stamp);



const char* asapo_message_data_get_as_chars(const AsapoMessageDataHandle data);

#endif
