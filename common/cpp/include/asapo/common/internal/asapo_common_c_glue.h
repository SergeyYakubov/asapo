#ifndef __COMMON_C_GLUE_H__
#define __COMMON_C_GLUE_H__
#include <memory>
#include "asapo/common/data_structs.h"
#include "asapo/common/error.h"
#define __COMMON_C_INTERFACE_IMPLEMENTATION__

class AsapoHandle {
  public:
    virtual ~AsapoHandle() {};
};

template<class T>
class AsapoHandlerHolder final : public AsapoHandle {
  public:
    AsapoHandlerHolder(bool manage_memory = true) : handle{nullptr}, manage_memory_{manage_memory} {};
    AsapoHandlerHolder(T* handle_i, bool manage_memory = true) : handle{handle_i}, manage_memory_{manage_memory} {};
    AsapoHandlerHolder(std::unique_ptr<T>& handle_i, bool manage_memory = true) : handle{handle_i.release()}, manage_memory_{manage_memory} {};
    ~AsapoHandlerHolder() override {
        if (!manage_memory_) {
            handle.release();
        }
    }
    std::unique_ptr<T> handle{nullptr};
  protected:
    bool manage_memory_{true};
};

template<>
class AsapoHandlerHolder < std::string>  final : public AsapoHandle {
  public:
    AsapoHandlerHolder(const std::string& handle_i) : handle{new std::string(handle_i)} {};
    ~AsapoHandlerHolder() override {
        handle.release();
    }
    std::unique_ptr<std::string> handle{nullptr};
};
//! handle for credentials to access a source from a producer
/// created by asapo_create_source_credentials()
/// free after use with asapo_free_handle()
/// \sa asapo::SourceCredentials
typedef AsapoHandlerHolder<asapo::SourceCredentials>* AsapoSourceCredentialsHandle;


//! handle for an asapo error
/// needs to be cleared after use with asapo_free_handle()
/// text version of an error: asapo_error_explain()
/// enum value of the error: asapo_error_get_type(), \sa ::AsapoErrorType asapo::ConsumerErrorType
typedef AsapoHandlerHolder<asapo::ErrorInterface>* AsapoErrorHandle;


//! handle for string return types
/// return type of several functions
/// free after use with asapo_free_handle()
/// a const pointer to the content can be obtained with asapo_string_c_str()
typedef AsapoHandlerHolder<std::string>* AsapoStringHandle;

//! handle for info about a stream,
/// may be set via asapo_stream_infos_get_info()
/// \sa asapo::StreamInfo asapo_stream_info_get_last_id() asapo_stream_info_get_name() asapo_stream_info_get_ffinished()  asapo_stream_info_get_next_stream()  asapo_stream_info_get_timestamp_created() asapo_stream_info_get_timestamp_last_entry()
typedef AsapoHandlerHolder<asapo::StreamInfo>* AsapoStreamInfoHandle;

//! handle for a set of stream infos
/// touch only with proper functions and use asapo_free_handle() to delete,
/// created by asapo_consumer_get_stream_list()
/// \sa asapo_free_handle() asapo_stream_infos_get_item() asapo_stream_infos_get_size()
typedef AsapoHandlerHolder<asapo::StreamInfos>* AsapoStreamInfosHandle;

//! handle for data recieved by the consumer
/// set as outout parameter via asapo_consumer_get_next(), asapo_consumer_get_last()
/// free after use with asapo_free_handle()
/// access to the data is granted via  asapo_message_data_get_as_chars()
typedef AsapoHandlerHolder<uint8_t[]>* AsapoMessageDataHandle;



int process_error(AsapoErrorHandle* error, asapo::Error err,
                  const asapo::ErrorTemplateInterface* p_exclude_err_template = nullptr);


AsapoHandle* handle_or_null(AsapoHandle* handle, AsapoErrorHandle* error, asapo::Error err,
                            const asapo::ErrorTemplateInterface* p_exclude_err_template = nullptr);

template <typename T> AsapoHandlerHolder<T>* handle_or_null_t(T* object,
        AsapoErrorHandle* error,
        asapo::Error err,
        const asapo::ErrorTemplateInterface* p_exclude_err_template = nullptr) {
    if (process_error(error, std::move(err), p_exclude_err_template) < 0) {
        return nullptr;
    } else {
        return new AsapoHandlerHolder<T>(object);
    }
}
AsapoHandlerHolder<std::string>* handle_or_null_t(const std::string& object,
                                                  AsapoErrorHandle* error,
                                                  asapo::Error err,
                                                  const asapo::ErrorTemplateInterface* p_exclude_err_template = nullptr) {
    if (process_error(error, std::move(err), p_exclude_err_template) < 0) {
        return nullptr;
    } else {
        return new AsapoHandlerHolder<std::string>(object);
    }
}



template<typename u, typename t>
constexpr bool operator==(const u& lhs, const t& rhs) {
    return static_cast<typename std::make_unsigned<u>::type>(lhs)
           == static_cast<typename std::make_unsigned<typename std::underlying_type<t>::type>::type>(rhs);
}



#endif
