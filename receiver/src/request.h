#ifndef HIDRA2_REQUEST_H
#define HIDRA2_REQUEST_H

#include "receiver_error.h"

namespace hidra2 {

class Request {
    Error Process();
    virtual uint64_t GetBodySize()=0;
};

}

#endif //HIDRA2_REQUEST_H
