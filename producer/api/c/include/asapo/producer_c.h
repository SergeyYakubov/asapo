#ifndef __PRODUCER_C_H__
#define __PRODUCER_C_H__
#include "asapo/common/common_c.h"
#ifndef __PRODUCER_C_INTERFACE_IMPLEMENTATION__

typedef struct {
    char _[AsapoHandleSize];
}* AsapoProducerHandle;
typedef struct {
    char _[AsapoHandleSize];
}* AsapoRequestCallbackPayloadHandle;
typedef struct {
    char _[AsapoHandleSize];
}* AsapoMessageHeaderHandle;
#endif

typedef void(*AsapoRequestCallback)(AsapoRequestCallbackPayloadHandle, AsapoErrorHandle);
#define kMaxMessageSize 1024
#define kMaxVersionSize 10
#define kNCustomParams 3

//! c version opf asapo::Opcode
enum AsapoOpcode {
    kOpcodeUnknownOp = 1,
    kOpcodeTransferData,
    kOpcodeTransferDatasetData,
    kOpcodeStreamInfo,
    kOpcodeLastStream,
    kOpcodeGetBufferData,
    kOpcodeAuthorize,
    kOpcodeTransferMetaData,
    kOpcodeDeleteStream,
    kOpcodeGetMeta,
    kOpcodeCount
};


//! c version of asapo::GenericRequestHeader
struct AsapoGenericRequestHeader {
    enum AsapoOpcode op_code;
    uint64_t    data_id;
    uint64_t    data_size;
    uint64_t    meta_size;
    uint64_t    custom_data[kNCustomParams];
    char        message[kMaxMessageSize]; /* Can also be a binary message (e.g. MemoryRegionDetails) */
    char        stream[kMaxMessageSize]; /* Must be a string (strcpy is used) */
    char        api_version[kMaxVersionSize]; /* Must be a string (strcpy is used) */
};

//! c version of asapo::RequestHandlerType
enum AsapoRequestHandlerType {
    kTcp,
    kFilesystem
};

//! c version of asapo::MetaIngestOp
enum AsapoMetaIngestOp {
    kInsert = 1,
    kReplace = 2,
    kUpdate = 3
};

//! c version of asapo::LogLevel
enum AsapoLogLevel {
    None,
    Error,
    Info,
    Debug,
    Warning
};



AsapoProducerHandle asapo_create_producer(const char* endpoint,
                                          uint8_t n_processing_threads,
                                          enum AsapoRequestHandlerType type,
                                          AsapoSourceCredentialsHandle source_cred,
                                          uint64_t timeout_ms,
                                          AsapoErrorHandle* error);
int asapo_producer_get_version_info(AsapoProducerHandle producer,
                                    AsapoStringHandle client_info,
                                    AsapoStringHandle server_info,
                                    AsapoBool* supported,
                                    AsapoErrorHandle* error);
AsapoStreamInfoHandle asapo_producer_get_stream_info(AsapoProducerHandle producer,
        const char* stream,
        uint64_t timeout_ms,
        AsapoErrorHandle* error);
AsapoStringHandle asapo_producer_get_stream_meta(AsapoProducerHandle producer,
                                                 const char* stream,
                                                 uint64_t timeout_ms,
                                                 AsapoErrorHandle* error);
AsapoStringHandle asapo_producer_get_beamtime_meta(AsapoProducerHandle producer,
        uint64_t timeout_ms,
        AsapoErrorHandle* error);
int asapo_producer_delete_stream(AsapoProducerHandle producer,
                                 const char* stream,
                                 uint64_t timeout_ms,
                                 AsapoBool delete_meta,
                                 AsapoBool error_on_not_exist,
                                 AsapoErrorHandle* error);
AsapoStreamInfoHandle asapo_producer_get_last_stream(AsapoProducerHandle producer,
        uint64_t timeout_ms,
        AsapoErrorHandle* error);

int asapo_producer_send(AsapoProducerHandle producer,
                        const AsapoMessageHeaderHandle message_header,
                        void* data,
                        uint64_t ingest_mode,
                        const char* stream,
                        AsapoRequestCallback callback,
                        AsapoErrorHandle* error);
int asapo_producer_send_file(AsapoProducerHandle producer,
                             const AsapoMessageHeaderHandle message_header,
                             const char* file_name,
                             uint64_t ingest_mode,
                             const char* stream,
                             AsapoRequestCallback callback,
                             AsapoErrorHandle* error);
int asapo_producer_send_stream_finished_flag(AsapoProducerHandle producer,
                                             const char* stream,
                                             uint64_t last_id,
                                             const char* next_stream,
                                             AsapoRequestCallback callback,
                                             AsapoErrorHandle* error);
int asapo_producer_send_beamtime_metadata(AsapoProducerHandle producer,
                                          const char* metadata,
                                          enum AsapoMetaIngestOp mode,
                                          AsapoBool upsert,
                                          AsapoRequestCallback callback,
                                          AsapoErrorHandle* error);
int asapo_producer_send_stream_metadata(AsapoProducerHandle producer,
                                        const char* metadata,
                                        enum AsapoMetaIngestOp mode,
                                        AsapoBool upsert,
                                        const char* stream,
                                        AsapoRequestCallback callback,
                                        AsapoErrorHandle* error);

AsapoMessageDataHandle asapo_request_callback_payload_get_data(AsapoRequestCallbackPayloadHandle handle);
AsapoStringHandle asapo_request_callback_payload_get_response(AsapoRequestCallbackPayloadHandle handle);
const struct AsapoGenericRequestHeader* asapo_request_callback_payload_get_original_header(
    AsapoRequestCallbackPayloadHandle handle);

void asapo_producer_set_log_level(AsapoProducerHandle producer, enum AsapoLogLevel level);
void asapo_producer_enable_local_log(AsapoProducerHandle producer, AsapoBool enable);
void asapo_producer_enable_remote_log(AsapoProducerHandle producer, AsapoBool enable);
int asapo_producer_set_credentials(AsapoProducerHandle producer, AsapoSourceCredentialsHandle source_cred,
                                   AsapoErrorHandle* error);
uint64_t  asapo_producer_get_requests_queue_size(AsapoProducerHandle producer);
uint64_t  asapo_producer_get_requests_queue_volume_mb(AsapoProducerHandle producer);
void asapo_producer_set_requests_queue_limits(AsapoProducerHandle producer, uint64_t size, uint64_t volume);
int asapo_producer_wait_requests_finished(AsapoProducerHandle producer, uint64_t timeout_ms,
                                          AsapoErrorHandle* error);

#endif
