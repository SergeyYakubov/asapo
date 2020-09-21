#ifndef ASAPO_ASAPO_WRAPPERS_H
#define ASAPO_ASAPO_WRAPPERS_H

#include <memory>
#include <functional>

namespace asapo {

inline std::string GetErrorString(asapo::Error* err) {
    if (*err) {
        return (*err)->Explain();
    }
    return "";
}

using RequestCallbackCython = void (*)(void*, void*, RequestCallbackPayload payload, Error err);
using RequestCallbackCythonMemory = void (*)(void*, void*, void*, RequestCallbackPayload payload, Error err);

RequestCallback unwrap_callback(RequestCallbackCython callback, void* c_self, void* py_func) {
    if (py_func == NULL) {
        return nullptr;
    }
    RequestCallback wrapper = [ = ](RequestCallbackPayload payload, Error err) -> void {
        callback(c_self, py_func, std::move(payload), std::move(err));
    };
    return wrapper;
}

RequestCallback unwrap_callback_with_memory(RequestCallbackCythonMemory callback, void* c_self, void* py_func,
                                            void* nd_array) {
    RequestCallback wrapper = [ = ](RequestCallbackPayload payload, Error err) -> void {
        callback(c_self, py_func, nd_array, std::move(payload), std::move(err));
    };
    return wrapper;
}

}


#endif //ASAPO_ASAPO_WRAPPERS_H
