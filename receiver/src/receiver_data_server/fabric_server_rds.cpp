#include <io/io_factory.h>

#include <utility>
#include "fabric_server_rds.h"
#include "receiver_data_server_logger.h"
#include "fabric_rds_request.h"

using namespace asapo;

FabricServerRds::FabricServerRds(std::string listenAddress): factory__(fabric::GenerateDefaultFabricFactory()), io__{GenerateDefaultIO()},
    log__{GetDefaultReceiverDataServerLogger()}, listenAddress_(std::move(listenAddress)) {

}

FabricServerRds::~FabricServerRds() {

}

Error FabricServerRds::Initialize() {
    if (server_) {
        return TextError("Server was already initialized");
    }
    Error err;
    std::string hostname;
    uint16_t port;
    std::tie(hostname, port) = *io__->SplitAddressToHostnameAndPort(listenAddress_);
    server_ = factory__->CreateAndBindServer(log__, hostname, port, &err);
    if (err) {
        return err;
    }

    return err;
}

GenericRequests FabricServerRds::GetNewRequests(Error* err) {
    // TODO: Should be performance tested, just a single request is returned at a time
    fabric::FabricAddress srcAddress;
    fabric::FabricMessageId messageId;

    GenericRequestHeader header;
    server_->RecvAny(&srcAddress, &messageId, &header, sizeof(header), err);
    if (err) {
        return {}; // empty result
    }
    auto requestPtr = new FabricRdsRequest(header, srcAddress, messageId);

    GenericRequests genericRequests;
    genericRequests.emplace_back(GenericRequestPtr(requestPtr));
    return genericRequests;
}

Error FabricServerRds::SendResponse(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response) {
    Error err;
    auto fabricRequest = dynamic_cast<const FabricRdsRequest*>(request);
    server_->Send(request->source_id, fabricRequest->message_id, response, sizeof(*response), &err);
}

Error FabricServerRds::SendResponseAndSlotData(const ReceiverDataServerRequest* request,
                                               const GenericNetworkResponse* response, const CacheMeta* cache_slot) {
    Error err;
    auto fabricRequest = dynamic_cast<const FabricRdsRequest*>(request);

    server_->RdmaWrite(fabricRequest->source_id, fabricRequest->GetMemoryRegion(), cache_slot->addr, cache_slot->size,
                       &err);
    if (err) {
        return err;
    }

    server_->Send(request->source_id, fabricRequest->message_id, response, sizeof(*response), &err);
    return err;
}

void FabricServerRds::HandleAfterError(uint64_t source_id) {
    /* Do nothing? */
}
