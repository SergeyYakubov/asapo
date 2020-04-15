#include <common/networking.h>
#include <io/io_factory.h>
#include "fabric_client.h"

using namespace asapo;

FabricClient::FabricClient(): factory__(fabric::GenerateDefaultFabricFactory()), io__{GenerateDefaultIO()} {

}

Error FabricClient::GetData(const FileInfo* info, FileData* data) {
    Error err;
    if (!client__) {
        client__ = factory__->CreateClient(&err);
        if (err) {
            return err;
        }
    }

    fabric::FabricAddress address = GetAddressOrConnect(info, &err);
    if (err) {
        return err;
    }

    /* MemoryRegion will be released when out of scope */
    auto mr = client__->ShareMemoryRegion(data->get(), info->size, &err);
    if (err) {
        return err;
    }

    GenericRequestHeader request_header{kOpcodeGetBufferData, info->buf_id, info->size};
    memcpy(request_header.message, mr->GetDetails(), sizeof(fabric::MemoryRegionDetails));

    auto currentMessageId = global_message_id_++;
    client__->Send(address, currentMessageId, &request_header, sizeof(request_header), &err);
    if (err) {
        return err;
    }

    /* The server is sending us the data over RDMA, and then sending us a confirmation */

    GenericNetworkResponse response{};
    client__->Recv(address, currentMessageId, &response, sizeof(response), &err);
    if (err) {
        return err;
    }

    if (response.error_code) {
        return TextError("Response NetworkErrorCode " + std::to_string(response.error_code));
    }

    return nullptr;
}

fabric::FabricAddress FabricClient::GetAddressOrConnect(const FileInfo* info, Error* error) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto tableEntry = known_addresses_.find(info->source);

    /* Check if we need to build up a connection */
    if (tableEntry == known_addresses_.end()) {
        fabric::FabricAddress address = client__->AddServerAddress(info->source, error);
        if (*error) {
            return -1;
        }
        return known_addresses_[info->source] = address;
    } else {
        return tableEntry->second;
    }
}
