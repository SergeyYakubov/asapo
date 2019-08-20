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

using cy_callback = void (*)(void*, GenericRequestHeader header, Error err);

class function_wrapper {
  public:
    static
    RequestCallback make_std_function(cy_callback callback, void* c_self)
    {
        RequestCallback wrapper = [=](GenericRequestHeader header, Error err) -> void
        {
          callback(c_self, header, std::move(err));
        };
        return wrapper;
    }
};

}


#endif //ASAPO_ASAPO_WRAPPERS_H
