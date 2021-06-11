#include <asapo/common/networking.h>
#include <asapo/io/io_factory.h>
#include <iostream>
#include "fabric_consumer_client.h"
#include "rds_response_error.h"
#include "asapo/common/internal/version.h"

using namespace asapo;

FabricConsumerClient::FabricConsumerClient(): factory__(fabric::GenerateDefaultFabricFactory()) {

}

Error FabricConsumerClient::GetData(const MessageMeta* info, MessageData* data) {
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

    MessageData tempData{new uint8_t[info->size]};

    /* MemoryRegion will be released when out of scope */
    auto mr = client__->ShareMemoryRegion(tempData.get(), info->size, &err);
    if (err) {
        return err;
    }

    GenericRequestHeader request_header{kOpcodeGetBufferData, info->buf_id, info->size};
    strncpy(request_header.receiver_protocol, kConsumerProtocol.GetRdsVersion().c_str(), kMaxVersionSize);
    memcpy(request_header.message, mr->GetDetails(), sizeof(fabric::MemoryRegionDetails));
    GenericNetworkResponse response{};

    PerformNetworkTransfer(address, &request_header, &response, &err);
    if (err) {
        return err;
    }

    if (response.error_code) {
        return ConvertRdsResponseToError(response.error_code);
    }

    data->swap(tempData);

    return nullptr;
}

fabric::FabricAddress FabricConsumerClient::GetAddressOrConnect(const MessageMeta* info, Error* error) {
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

void FabricConsumerClient::PerformNetworkTransfer(fabric::FabricAddress address,
                                                  const GenericRequestHeader* request_header,
                                                  GenericNetworkResponse* response, Error* err) {
    auto currentMessageId = global_message_id_++;
    client__->Send(address, currentMessageId, request_header, sizeof(*request_header), err);
    if (*err) {
        return;
    }

    /* The server is _now_ sending us the data over RDMA, and then sending us a confirmation */

    client__->Recv(address, currentMessageId, response, sizeof(*response), err);
    // if (*err) ...
}
