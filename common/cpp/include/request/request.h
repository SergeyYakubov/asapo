#ifndef ASAPO_GENERIC_REQUEST_H
#define ASAPO_GENERIC_REQUEST_H

#include "common/networking.h"
#include "common/data_structs.h"
#include "io/io.h"

namespace asapo {

class GenericRequest {
 public:
    GenericRequest() = delete;
    GenericRequest(GenericRequestHeader h): header{std::move(h)}{};
    GenericRequestHeader header;
    virtual ~GenericRequest() = default;
};

}

#endif //ASAPO_GENERIC_REQUEST_H
