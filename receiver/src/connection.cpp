#include <cstring>
#include <assert.h>
#include "connection.h"
#include "receiver_error.h"
#include "asapo/io/io_factory.h"

#include "receiver_logger.h"

namespace asapo {

Connection::Connection(SocketDescriptor socket_fd, const std::string& address,
                       SharedCache cache, std::string receiver_tag) :
    io__{GenerateDefaultIO()},
    statistics__{new ReceiverStatistics},
             log__{GetDefaultReceiverLogger()},
requests_dispatcher__{new RequestsDispatcher{socket_fd, address, statistics__.get(), cache}}  {
    socket_fd_ = socket_fd;
    address_ = address;
    statistics__->AddTag("connection_from", address);
    statistics__->AddTag("receiver_tag", std::move(receiver_tag));
}



void Connection::ProcessStatisticsAfterRequest(const std::unique_ptr<Request>& request) const noexcept {
    statistics__->IncreaseRequestCounter();
    statistics__->IncreaseRequestDataVolume(request->GetDataSize() + sizeof(GenericRequestHeader) +
                                            sizeof(GenericNetworkResponse));
    statistics__->SendIfNeeded();
}

void Connection::Listen() const noexcept {
    while (true) {
        Error err;
        auto request = requests_dispatcher__->GetNextRequest(&err);
        if (err) {
            break;
        }
        err = requests_dispatcher__->ProcessRequest(request);
        if (err) {
            break;
        }
        ProcessStatisticsAfterRequest(request);
    }
    io__->CloseSocket(socket_fd_, nullptr);
    statistics__->SendIfNeeded(true);
    log__->Info(LogMessageWithFields("disconnected from ").Append("origin", HostFromUri(address_)));
}


}


