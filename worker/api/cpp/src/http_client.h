#ifndef HIDRA2_HTTP_CLIENT_H
#define HIDRA2_HTTP_CLIENT_H

#include <hidra2_worker.h>

namespace hidra2 {

class HttpClient {
  public:
    virtual std::string Get(const std::string& uri, WorkerErrorCode* err) const = 0;
};

}

#endif //HIDRA2_HTTP_CLIENT_H
