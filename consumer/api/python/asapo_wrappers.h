#ifndef ASAPO_ASAPO_WRAPPERS_H
#define ASAPO_ASAPO_WRAPPERS_H

#include <string>

namespace asapo {

inline std::string GetErrorString(asapo::Error* err) {
    if (*err) {
        return (*err)->Explain();
    }
    return "";
}

}


#endif //ASAPO_ASAPO_WRAPPERS_H
