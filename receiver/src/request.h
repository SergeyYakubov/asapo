#ifndef HIDRA2_REQUEST_H
#define HIDRA2_REQUEST_H

#include "receiver_error.h"
#include "common/networking.h"
#include "system_wrappers/io.h"
#include "request_handler.h"
namespace hidra2 {

class RequestHandler;

class Request {
  public:
    virtual Error Handle();
    virtual ~Request() = default;
    Request(const GenericNetworkRequestHeader& request_header, SocketDescriptor socket_fd);
    void AddHandler(const RequestHandler*);
    std::unique_ptr<IO> io__;
  private:
    Error AllocateDataBuffer();
    Error ReceiveData();
    const GenericNetworkRequestHeader request_header_;
    const SocketDescriptor socket_fd_;
    FileData data_buffer_;
    std::vector<const RequestHandler*> handlers_;
};

class RequestFactory {
  public:
    virtual std::unique_ptr<Request> GenerateRequest(const GenericNetworkRequestHeader& request_header,
                                                     SocketDescriptor socket_fd, Error* err) const noexcept ;
};

}

#endif //HIDRA2_REQUEST_H
