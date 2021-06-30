#include "asapo/asapo_fabric/asapo_fabric.h"
#include "fabric_factory_not_supported.h"

#ifdef LIBFABRIC_ENABLED
#include <dlfcn.h>
#include <mutex>
#include <iostream>
#include "fabric_factory_impl.h"
#include "fabric_function_map.h"
#endif

using namespace asapo::fabric;

std::unique_ptr<FabricFactory> asapo::fabric::GenerateDefaultFabricFactory() {
#ifdef LIBFABRIC_ENABLED
    static std::mutex lock;
    std::unique_lock<std::mutex> local_lock (lock);

    if (gffm().is_init_) {
        return std::unique_ptr<FabricFactory>(new FabricFactoryImpl());
    }

    const char* libfabric_path_override = std::getenv("ASAPO_LIBFABRIC_LIBRARY");
    void* handle = nullptr;
    if (libfabric_path_override) {
        handle = dlopen(libfabric_path_override, RTLD_LAZY);
        if (!handle) {
            std::cerr << "WARN: 'ASAPO_LIBFABRIC_LIBRARY' was set, but failed to open. Reason: '" << dlerror() <<
                      "'. Fallback to normal path." << std::endl;
        }
    }
    if (!handle) {
        handle = dlopen("libfabric.so.1", RTLD_LAZY); // Try release v1.xx
    }
    if (!handle) {
        return std::unique_ptr<FabricFactory>(new FabricFactoryNotSupported(FabricErrorTemplates::kLibraryNotFoundError));
    }

#define ADD_FABRIC_CALL(fName) do { if (!(*((void**)&gffm().fName) = dlsym(handle, #fName))) goto functionNotFoundError; } while(0)
    ADD_FABRIC_CALL(fi_version);
    ADD_FABRIC_CALL(fi_dupinfo);
    ADD_FABRIC_CALL(fi_freeinfo);
    ADD_FABRIC_CALL(fi_getinfo);
    ADD_FABRIC_CALL(fi_fabric);
    ADD_FABRIC_CALL(fi_strerror);
#undef ADD_FABRIC_CALL

    gffm().is_init_ = true;

    return std::unique_ptr<FabricFactory>(new FabricFactoryImpl());
functionNotFoundError:
    dlclose(handle);
    return std::unique_ptr<FabricFactory>(new FabricFactoryNotSupported(FabricErrorTemplates::kLibraryCompatibilityError));
#endif
    return std::unique_ptr<FabricFactory>(new FabricFactoryNotSupported(FabricErrorTemplates::kNotSupportedOnBuildError));
}

#ifdef LIBFABRIC_ENABLED
// Global fabric function map
extern FabricFunctionMap& gffm() {
    static FabricFunctionMap gffm_;
    return gffm_;
}
#endif
