#ifndef HIDRA2_SEND_DATA_REQUEST_H
#define HIDRA2_SEND_DATA_REQUEST_H

#include "request.h"
#include "common/networking.h"
#include "system_wrappers/io.h"

namespace hidra2 {

class SendDataRequest : public Request {
  public:
    SendDataRequest(const std::unique_ptr<GenericNetworkRequestHeader>& request_header, SocketDescriptor socket_fd);

  private:
    GenericNetworkRequestHeader request_header_;
};

}

#endif //HIDRA2_SEND_DATA_REQUEST_H
