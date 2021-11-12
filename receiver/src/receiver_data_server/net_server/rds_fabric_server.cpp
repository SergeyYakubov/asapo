#include "asapo/io/io_factory.h"

#include <utility>
#include "rds_fabric_server.h"
#include "../receiver_data_server_logger.h"
#include "fabric_rds_request.h"

using namespace asapo;

RdsFabricServer::RdsFabricServer(std::string listenAddress,
                                 const AbstractLogger* logger): factory__(fabric::GenerateDefaultFabricFactory()), io__{GenerateDefaultIO()},
    log__{logger}, listenAddress_(std::move(listenAddress)) {

}

RdsFabricServer::~RdsFabricServer() {

}

Error RdsFabricServer::Initialize() {
    if (server__) {
        return GeneralErrorTemplates::kSimpleError.Generate("Server was already initialized");
    }
    Error err;
    std::string hostname;
    uint16_t port;
    std::tie(hostname, port) = *io__->SplitAddressToHostnameAndPort(listenAddress_);
    server__ = factory__->CreateAndBindServer(log__, hostname, port, &err);
    if (err) {
        return err;
    }

    log__->Info(LogMessageWithFields("started fabric data server").Append("address",server__->GetAddress()));

    return err;
}

GenericRequests RdsFabricServer::GetNewRequests(Error* err) {
    // TODO: Should be performance tested, just a single request is returned at a time
    fabric::FabricAddress srcAddress;
    fabric::FabricMessageId messageId;

    GenericRequestHeader header;
    server__->RecvAny(&srcAddress, &messageId, &header, sizeof(header), err);
    if (*err) {
        return {}; // empty result
    }
    auto requestPtr = new FabricRdsRequest(header, srcAddress, messageId);

    GenericRequests genericRequests;
    genericRequests.emplace_back(GenericRequestPtr(requestPtr));
    return genericRequests;
}

Error RdsFabricServer::SendResponse(const ReceiverDataServerRequest* request, const GenericNetworkResponse* response) {
    Error err;
    auto fabricRequest = dynamic_cast<const FabricRdsRequest*>(request);
    server__->Send(request->source_id, fabricRequest->message_id, response, sizeof(*response), &err);
    return err;
}

Error RdsFabricServer::SendResponseAndSlotData(const ReceiverDataServerRequest* request,
                                               const GenericNetworkResponse* response, const CacheMeta* cache_slot) {
    Error err;
    auto fabricRequest = dynamic_cast<const FabricRdsRequest*>(request);

    server__->RdmaWrite(fabricRequest->source_id, fabricRequest->GetMemoryRegion(), cache_slot->addr, cache_slot->size,
                        &err);
    if (err) {
        return err;
    }

    server__->Send(request->source_id, fabricRequest->message_id, response, sizeof(*response), &err);
    return err;
}

void RdsFabricServer::HandleAfterError(uint64_t) {
    /* Do nothing? */
}
