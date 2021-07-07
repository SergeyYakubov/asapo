#ifndef __PRODUCER_C_H__
#define __PRODUCER_C_H__
#include "asapo/common/common_c.h"
#ifndef __PRODUCER_C_INTERFACE_IMPLEMENTATION__

typedef void* AsapoProducerHandle;
typedef void* AsapoRequestCallbackPayloadHandle;
typedef void* AsapoMessageHeaderHandle;
#endif

typedef void(*AsapoRequestCallback)(AsapoRequestCallbackPayloadHandle, AsapoErrorHandle);


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

AsapoProducerHandle asapo_create_producer(const char* endpoint,
                                          uint8_t n_processing_threads,
                                          AsapoRequestHandlerType type,
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
                        AsapoMessageDataHandle data,
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
                                          AsapoMetaIngestOp mode,
                                          AsapoBool upsert,
                                          AsapoRequestCallback callback,
                                          AsapoErrorHandle* error);
int asapo_producer_send_stream_metadata(AsapoProducerHandle producer,
                                        const char* metadata,
                                        AsapoMetaIngestOp mode,
                                        AsapoBool upsert,
                                        const char* stream,
                                        AsapoRequestCallback callback,
                                        AsapoErrorHandle* error);

#endif
