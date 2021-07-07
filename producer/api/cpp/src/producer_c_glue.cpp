#define __PRODUCER_C_INTERFACE_IMPLEMENTATION__
#include "asapo/common/internal/asapo_common_c_glue.h"
#include "asapo/asapo_producer.h"


//! handle for an asapo producer
/// created by asapo_create_producer()
/// free after use with asapo_free_handle()
/// all operations are done with asapo_producer_xxx() functions
/// \sa asapo::Producer
typedef AsapoHandlerHolder<asapo::Producer>* AsapoProducerHandle;
typedef AsapoHandlerHolder<asapo::RequestCallbackPayload>* AsapoRequestCallbackPayloadHandle;
typedef AsapoHandlerHolder<asapo::MessageHeader>* AsapoMessageHeaderHandle;

extern "C" {
#include "asapo/producer_c.h"
    AsapoProducerHandle asapo_create_producer(const char* endpoint,
                                              uint8_t n_processing_threads,
                                              AsapoRequestHandlerType type,
                                              AsapoSourceCredentialsHandle source_cred,
                                              uint64_t timeout_ms,
                                              AsapoErrorHandle* error) {
        asapo::Error err;
        auto c = asapo::Producer::Create(endpoint,
                                         n_processing_threads,
                                         static_cast<asapo::RequestHandlerType>(type),
                                         *(source_cred->handle),
                                         timeout_ms,
                                         &err);
        return handle_or_null_t(c.release(), error, std::move(err));

    }
    int asapo_producer_get_version_info(AsapoProducerHandle producer,
                                        AsapoStringHandle client_info,
                                        AsapoStringHandle server_info,
                                        AsapoBool* supported,
                                        AsapoErrorHandle* error) {
        bool supp;
        auto err = producer->handle->GetVersionInfo(client_info->handle.get(),
                                                    server_info->handle.get(),
                                                    &supp);
        *supported = supp;
        return process_error(error, std::move(err));
    }

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

#define BUILD_WRAPPER asapo::RequestCallback wrapper = [ = ](asapo::RequestCallbackPayload payload, asapo::Error err) -> void { \
            auto payLoadHandle = new AsapoHandlerHolder<asapo::RequestCallbackPayload>(&payload); \
            auto errorHandle = new AsapoHandlerHolder<asapo::ErrorInterface>(err.release()); \
            callback(payLoadHandle, errorHandle); \
            delete errorHandle; \
            delete payLoadHandle; \
        }

    int asapo_producer_send(AsapoProducerHandle producer,
                            const AsapoMessageHeaderHandle message_header,
                            AsapoMessageDataHandle data,
                            uint64_t ingest_mode,
                            const char* stream,
                            AsapoRequestCallback callback,
                            AsapoErrorHandle* error) {
        BUILD_WRAPPER;
        auto err = producer->handle->Send(*message_header->handle,
                                          std::move(data->handle),
                                          ingest_mode,
                                          stream,
                                          wrapper);
        return process_error(error, std::move(err));
    }
    int asapo_producer_send_file(AsapoProducerHandle producer,
                                 const AsapoMessageHeaderHandle message_header,
                                 const char* file_name,
                                 uint64_t ingest_mode,
                                 const char* stream,
                                 AsapoRequestCallback callback,
                                 AsapoErrorHandle* error) {
        BUILD_WRAPPER;
        auto err = producer->handle->SendFile(*message_header->handle,
                                              file_name,
                                              ingest_mode,
                                              stream,
                                              wrapper);
        return process_error(error, std::move(err));
    }
    int asapo_producer_send_stream_finished_flag(AsapoProducerHandle producer,
                                                 const char* stream,
                                                 uint64_t last_id,
                                                 const char* next_stream,
                                                 AsapoRequestCallback callback,
                                                 AsapoErrorHandle* error) {
        BUILD_WRAPPER;
        auto err = producer->handle->SendStreamFinishedFlag(stream,
                   last_id,
                   next_stream,
                   wrapper);
        return process_error(error, std::move(err));
    }
    int asapo_producer_send_beamtime_metadata(AsapoProducerHandle producer,
                                              const char* metadata,
                                              AsapoMetaIngestOp mode,
                                              AsapoBool upsert,
                                              AsapoRequestCallback callback,
                                              AsapoErrorHandle* error) {
        BUILD_WRAPPER;
        asapo::MetaIngestMode im(static_cast<asapo::MetaIngestOp>(mode), upsert != 0);
        auto err = producer->handle->SendBeamtimeMetadata(metadata,
                                                          im,
                                                          wrapper);
        return process_error(error, std::move(err));

    }
    int asapo_producer_send_stream_metadata(AsapoProducerHandle producer,
                                            const char* metadata,
                                            AsapoMetaIngestOp mode,
                                            AsapoBool upsert,
                                            const char* stream,
                                            AsapoRequestCallback callback,
                                            AsapoErrorHandle* error) {
        BUILD_WRAPPER;
        asapo::MetaIngestMode im(static_cast<asapo::MetaIngestOp>(mode), upsert != 0);
        auto err = producer->handle->SendStreamMetadata(metadata,
                                                        im,
                                                        stream,
                                                        wrapper);
        return process_error(error, std::move(err));

    }




}
