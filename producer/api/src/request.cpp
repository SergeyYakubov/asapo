#include <producer/producer_error.h>
#include "request.h"

namespace asapo {

Request::Request(const asapo::IO* io, const GenericNetworkRequestHeader& header, const void* data,
                 RequestCallback callback):
    io__{io}, header_{header}, data_{data}, callback_{std::move(callback)} {

}

Error Request::ConnectToReceiver(SocketDescriptor* sd, const std::string& receiver_address) {
    Error err;
    *sd = io__->CreateAndConnectIPTCPSocket(receiver_address, &err);
    if(err != nullptr) {
        //log__->Debug("cannot connect to receiver at " + receiver_address + " - " + err->Explain());
        return err;
    }
    return nullptr;
}

Error Request::SendHeaderAndData(SocketDescriptor sd) {
    Error io_error;
    io__->Send(sd, &header_, sizeof(header_), &io_error);
    if(io_error) {
// todo: add meaningful message to the io_error (here and below)
        return io_error;
    }

    io__->Send(sd, data_, header_.data_size, &io_error);
    if(io_error) {
        return io_error;
    }

    return nullptr;
}


Error Request::ReceiveResponce(SocketDescriptor sd) {
    Error err;
    SendDataResponse sendDataResponse;
    io__->Receive(sd, &sendDataResponse, sizeof(sendDataResponse), &err);
    if(err != nullptr) {
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


Error Request::Send(SocketDescriptor* sd, const ReceiversList& receivers_list) {
    for (auto receiver_uri : receivers_list) {
        if (*sd == kDisconnectedSocketDescriptor) {
            auto err = ConnectToReceiver(sd, receiver_uri);
            if (err != nullptr ) continue;
        }

        auto err = SendHeaderAndData(*sd);
        if (err != nullptr )  {
            io__->CloseSocket(*sd, nullptr);
            *sd = kDisconnectedSocketDescriptor;
            continue;
        }

        err = ReceiveResponce(*sd);

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

}