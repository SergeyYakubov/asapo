#include "tcp_client.h"
#include "io/io_factory.h"
#include "common/networking.h"
namespace asapo {

TcpClient::TcpClient() : io__{GenerateDefaultIO()}, connection_pool__{new TcpConnectionPool()} {

}


Error TcpClient::SendGetDataRequest(SocketDescriptor sd, const FileInfo* info) const noexcept {
    Error err;
    GenericRequestHeader request_header{kOpcodeGetBufferData, info->buf_id, info->size};
    io__->Send(sd, &request_header, sizeof(request_header), &err);
    if (err) {
        io__->CloseSocket(sd, nullptr);
    }
    return err;
}

Error TcpClient::ReconnectAndResendGetDataRequest(SocketDescriptor* sd, const FileInfo* info) const noexcept {
    Error err;
    *sd = connection_pool__->Reconnect(*sd, &err);
    if (err) {
        return err;
    } else {
        return SendGetDataRequest(*sd, info);
    }
}

Error TcpClient::ReceiveResponce(SocketDescriptor sd) const noexcept {
    Error err;

    GenericNetworkResponse Response;
    io__->Receive(sd, &Response, sizeof(Response), &err);
    if(err != nullptr) {
        io__->CloseSocket(sd, nullptr);
        return err;
    }
    switch (Response.error_code) {
    case kNetErrorWrongRequest :
        io__->CloseSocket(sd, nullptr);
        return Error{new SimpleError("internal server error: wrong request")};
    case kNetErrorNoData :
        return Error{new SimpleError("no data")};
    default:
        return nullptr;
    }
}

Error TcpClient::QueryCacheHasData(SocketDescriptor* sd, const FileInfo* info, bool try_reconnect) const noexcept {
    Error err;
    err = SendGetDataRequest(*sd, info);
    if (err && try_reconnect) {
        err = ReconnectAndResendGetDataRequest(sd, info);
    }
    if (err) {
        return err;
    }

    return ReceiveResponce(*sd);
}

Error TcpClient::ReceiveData(SocketDescriptor sd, const FileInfo* info, FileData* data) const noexcept {
    Error err;
    uint8_t* data_array = nullptr;
    try {
        data_array = new uint8_t[info->size];
    } catch (...) {
        return ErrorTemplates::kMemoryAllocationError.Generate();
    }
    io__->Receive(sd, data_array, info->size, &err);
    if (!err) {
        *data = FileData{data_array};
    } else {
        delete[] data_array;
    }
    return err;
}


Error TcpClient::GetData(const FileInfo* info, FileData* data) const noexcept {
    Error err;
    bool reused;
    auto sd = connection_pool__->GetFreeConnection(info->source, &reused, &err);
    if (err != nullptr) {
        return err;
    }

    err = QueryCacheHasData(&sd, info, reused);
    if (err) {
        return err;
    }

    return ReceiveData(sd, info, data);
}

}