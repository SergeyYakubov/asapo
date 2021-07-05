#define __COMMON_C_INTERFACE_IMPLEMENTATION__
#include "asapo/asapo_common_c_glue.h

//! handle for credentials to access a source from a producer
/// created by asapo_create_source_credentials()
/// free after use with asapo_free_handle()
/// \sa asapo::SourceCredentials
typedef AsapoHandlerHolder<asapo::SourceCredentials>* AsapoSourceCredentialsHandle;

extern "C" {

//! free handle memory, set handle to NULL
/// \param[in] pointer to an ASAPO handle
    void asapo_free_handle(void** handle) {
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

}
