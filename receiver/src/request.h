#ifndef HIDRA2_REQUEST_H
#define HIDRA2_REQUEST_H

#include "receiver_error.h"
#include "common/networking.h"
#include "system_wrappers/io.h"

namespace hidra2 {

class Request {
  public:
    virtual Error Handle();
    ~Request() = default;
    Request(const std::unique_ptr<GenericNetworkRequestHeader>& request_header, SocketDescriptor socket_fd);
    std::unique_ptr<IO> io__;
  private:
    const GenericNetworkRequestHeader request_header_;
    const SocketDescriptor socket_fd_;
    FileData data_buffer_;
};

class RequestFactory {
  public:
    virtual std::unique_ptr<Request> GenerateRequest(const std::unique_ptr<GenericNetworkRequestHeader>& request_header,
                                                     SocketDescriptor socket_fd, Error* err) const noexcept ;
};

}

#endif //HIDRA2_REQUEST_H
