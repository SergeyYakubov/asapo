#define __COMMON_C_INTERFACE_IMPLEMENTATION__
#include "asapo/common/internal/asapo_common_c_glue.h"
#include <algorithm>

AsapoHandlerHolder<std::string>* handle_or_null_t(const std::string& object,
                                                  AsapoErrorHandle* error,
                                                  asapo::Error err,
                                                  const asapo::ErrorTemplateInterface* p_exclude_err_template) {
    if (process_error(error, std::move(err), p_exclude_err_template) < 0) {
        return nullptr;
    } else {
        return new AsapoHandlerHolder<std::string>(object);
    }
}



int process_error(AsapoErrorHandle* error, asapo::Error err,
                  const asapo::ErrorTemplateInterface* p_exclude_err_template) {
    int retval = (err == nullptr || (p_exclude_err_template != nullptr && err == *p_exclude_err_template)) ? 0 : -1;
    if (error == nullptr) {
        return retval;
    }
    if (*error == nullptr) {
        *error = new AsapoHandlerHolder<asapo::ErrorInterface> {err.release()};
    } else {
        (*error)->handle = std::move(err);
    }
    return retval;
}


void time_point_to_time_spec(std::chrono::system_clock::time_point tp,
                             struct timespec* stamp) {
    stamp->tv_sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    stamp->tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count() % 1000000000;
}



extern "C" {
#include "asapo/common/common_c.h"
    static_assert(kProcessed == asapo::SourceType::kProcessed&&
                  kRaw == asapo::SourceType::kRaw,
                  "incompatible bit reps between c++ and c for asapo::SourceType");

    static_assert(AsapoHandleSize == sizeof(AsapoHandlerHolder<int>),
                  "AsapoHandleSize is not correct");


    AsapoStringHandle asapo_string_from_c_str(const char* str) {
        return new AsapoHandlerHolder<std::string> {std::string{str}};
    }



//! free handle memory, set handle to NULL
/// \param[in] pointer to an ASAPO handle
    void asapo_free_handle__(void** handle) {
        if (*handle == nullptr) {
            return;
        }
        auto a_handle = static_cast<AsapoHandle*>(*handle);
        delete a_handle;
        *handle = nullptr;
    }

//! creates a new ASAPO handle
/// \return initialized ASAPO handle
    void* asapo_new_handle() {
        return NULL;
    }

/// \copydoc asapo::ErrorInterface::Explain()
/// \param[out] buf will be filled with the explanation
/// \param[in] maxSize max size of buf in bytes
    void asapo_error_explain(const AsapoErrorHandle error, char* buf, size_t maxSize) {
        if (error->handle) {
            const auto& msg = error->handle->Explain();
            std::copy_n(msg.begin(), std::max(msg.size(), maxSize), buf);
            buf[std::max(maxSize - 1, msg.size())] = '\0';
        } else {
            static std::string msg("no error");
            std::copy_n(msg.begin(), std::max(msg.size(), maxSize), buf);
            buf[std::max(maxSize - 1, msg.size())] = '\0';
        }
    }


    //! give a pointer to the content of the asapoString
/// \param[in] str the handle of the asapoString in question
/// \return const char pointer to the content
    const char* asapo_string_c_str(const AsapoStringHandle str) {
        return str->handle->c_str();
    }
//! give the size of an asapoString
/// \param[in] str the handle of the asapoString in question
/// \return the number of bytes in the string , not counting the final nul byte.
    size_t asapo_string_size(const AsapoStringHandle str) {
        return str->handle->size();
    }

//! get one stream info from a stream infos handle
/// \param[in] infos handle for stream infos
/// \param[in] index index od info to get, starts at 0
/// \return handle to stream info
    AsapoStreamInfoHandle asapo_stream_infos_get_item(const AsapoStreamInfosHandle infos,
                                                      size_t index) {
        return new AsapoHandlerHolder<asapo::StreamInfo> {&(infos->handle->at(index)), false};
    }

//! get size (number of elements) of a stream infos handle
/// \param[in] infos handle for stream infos
/// \return number of elements in the handle
    size_t asapo_stream_infos_get_size(const AsapoStreamInfosHandle infos) {
        return infos->handle->size();
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
    const char* asapo_stream_info_get_name(const AsapoStreamInfoHandle info) {
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
    const char* asapo_stream_info_get_next_stream(const AsapoStreamInfoHandle info) {
        return info->handle->next_stream.c_str();
    }
//! get creation time from the stream info object
/// \param[in] info handle of the stream info object
/// \param[out] stamp creation timestamp as timespec
/// \sa asapo::StreamInfo
    void asapo_stream_info_get_timestamp_created(const AsapoStreamInfoHandle info,
                                                 struct timespec* stamp) {
        time_point_to_time_spec(info->handle->timestamp_created, stamp);
    }
//! get time of last entry from the stream info object
/// \param[in] info handle of the stream info object
/// \param[out] stamp last entry timestamp as timespec
/// \sa asapo::StreamInfo
    void asapo_stream_info_get_timestamp_last_entry(const AsapoStreamInfoHandle info,
                                                    struct timespec* stamp) {
        time_point_to_time_spec(info->handle->timestamp_lastentry, stamp);
    }

//! give acess to data
/// \param[in] data the handle of the data
/// \return const char pointer to the data blob, valid until deletion or reuse of data
    const char* asapo_message_data_get_as_chars(const AsapoMessageDataHandle data) {
        return reinterpret_cast<const char*>(data->handle.get());
    }

//! wraps asapo::SourceCredentials::SourceCredentials()
/// \copydoc asapo::SourceCredentials::SourceCredentials()
    AsapoSourceCredentialsHandle asapo_create_source_credentials(enum AsapoSourceType type,
            const char* instanceId,
            const char* pipelineStep,
            const char* beamtime,
            const char* beamline,
            const char* data_source,
            const char* token) {
        auto retval = new asapo::SourceCredentials(static_cast<asapo::SourceType>(type),
                                                   instanceId, pipelineStep,
                                                   beamtime, beamline,
                                                   data_source, token);
        return new AsapoHandlerHolder<asapo::SourceCredentials> {retval};
    }


    AsapoBool asapo_is_error(AsapoErrorHandle err) {
        return err != nullptr && err->handle != nullptr;
    }

}
