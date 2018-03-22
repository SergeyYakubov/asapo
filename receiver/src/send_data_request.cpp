#include "send_data_request.h"

namespace hidra2 {

SendDataRequest::SendDataRequest(const std::unique_ptr<GenericNetworkRequestHeader>& header,
                                 SocketDescriptor socket_fd) : Request(header, socket_fd) {

}

}