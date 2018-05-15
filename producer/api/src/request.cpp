#include "producer/producer_error.h"
#include "request.h"
#include "producer_logger.h"

namespace asapo {

Request::Request(const asapo::IO* io, const GenericNetworkRequestHeader& header, const void* data,
                 RequestCallback callback):
    io__{io}, log__{GetDefaultProducerLogger()}, header_(header), data_{data}, callback_{std::move(callback)} {

}

Error Request::ConnectToReceiver(SocketDescriptor* sd, const std::string& receiver_address) {
    Error err;
    *sd = io__->CreateAndConnectIPTCPSocket(receiver_address, &err);
    if(err != nullptr) {
        //log__->Debug("cannot connect to receiver at " + receiver_address + " - " + err->Explain());
        return err;
    }
    log__->Info("connected to receiver at " + receiver_address);
    return nullptr;
}

Error Request::SendHeaderAndData(SocketDescriptor sd, const std::string& receiver_address) {
    Error io_error;
    io__->Send(sd, &header_, sizeof(header_), &io_error);
    if(io_error) {
// todo: add meaningful message to the io_error (here and below)
        log__->Debug("cannot send header to " + receiver_address + " - " + io_error->Explain());
        return io_error;
    }

    io__->Send(sd, data_, header_.data_size, &io_error);
    if(io_error) {
        log__->Debug("cannot send data to " + receiver_address + " - " + io_error->Explain());
        return io_error;
    }

    return nullptr;
}


Error Request::ReceiveResponse(SocketDescriptor sd, const std::string& receiver_address) {
    Error err;
    SendDataResponse sendDataResponse;
    io__->Receive(sd, &sendDataResponse, sizeof(sendDataResponse), &err);
    if(err != nullptr) {
        log__->Debug("cannot receive response from " + receiver_address + " - " + err->Explain());
        return err;
    }

    if(sendDataResponse.error_code) {
        if(sendDataResponse.error_code == kNetErrorFileIdAlreadyInUse) {
            return ProducerErrorTemplates::kFileIdAlreadyInUse.Generate();
        }
        return ProducerErrorTemplates::kUnknownServerError.Generate();
    }
    return nullptr;
}

Error Request::TrySendToReceiver(SocketDescriptor sd, const std::string& receiver_address) {
    auto err = SendHeaderAndData(sd, receiver_address);
    if (err)  {
        return err;
    }

    err = ReceiveResponse(sd, receiver_address);
    if (err)  {
        return err;
    }

    log__->Debug("successfully sent data to " + receiver_address);
    return nullptr;
}



Error Request::Send(SocketDescriptor* sd, const ReceiversList& receivers_list,bool rebalance) {
    if (rebalance && *sd != kDisconnectedSocketDescriptor) {
        io__->CloseSocket(*sd, nullptr);
        log__->Info("rebalancing");
        *sd = kDisconnectedSocketDescriptor;
    }
    for (auto receiver_uri : receivers_list) {
        if (*sd == kDisconnectedSocketDescriptor) {
            auto err = ConnectToReceiver(sd, receiver_uri);
            if (err != nullptr ) continue;
        }

        auto err = TrySendToReceiver(*sd, receiver_uri);
        if (err != nullptr && err != ProducerErrorTemplates::kFileIdAlreadyInUse)  {
            io__->CloseSocket(*sd, nullptr);
            *sd = kDisconnectedSocketDescriptor;
            continue;
        }

        callback_(header_, std::move(err));
        return nullptr;

    }
    return ProducerErrorTemplates::kCannotSendDataToReceivers.Generate();
}

uint64_t Request::GetMemoryRequitements() {
    return header_.data_size + sizeof(Request);
}


}
