#include "producer/producer_error.h"
#include "request_handler_tcp.h"
#include "producer_logger.h"
#include "io/io_factory.h"


namespace asapo {


RequestHandlerTcp::RequestHandlerTcp(ReceiverDiscoveryService* discovery_service, uint64_t thread_id,
                                     uint64_t* shared_counter):
    io__{GenerateDefaultIO()}, log__{GetDefaultProducerLogger()}, discovery_service__{discovery_service}, thread_id_{thread_id},
    ncurrent_connections_{shared_counter} {

}

Error RequestHandlerTcp::Authorize(const std::string& beamtime_id) {
    GenericRequestHeader header{kOpcodeAuthorize, 0, 0, beamtime_id.c_str()};
    Error err;
    io__->Send(sd_, &header, sizeof(header), &err);
    if(err) {
        return err;
    }
    return ReceiveResponse();
}


Error RequestHandlerTcp::ConnectToReceiver(const std::string& beamtime_id, const std::string& receiver_address) {
    Error err;

    sd_ = io__->CreateAndConnectIPTCPSocket(receiver_address, &err);
    if(err != nullptr) {
        log__->Debug("cannot connect to receiver at " + receiver_address + " - " + err->Explain());
        return err;
    }
    log__->Debug("connected to receiver at " + receiver_address);

    connected_receiver_uri_ = receiver_address;
    err = Authorize(beamtime_id);
    if (err != nullptr) {
        log__->Error("authorization failed at " + receiver_address + " - " + err->Explain());
        Disconnect();
        return err;
    }

    log__->Info("authorized connection to receiver at " + receiver_address);

    return nullptr;
}

Error RequestHandlerTcp::SendHeaderAndData(const Request* request) {
    Error io_error;
    io__->Send(sd_, &(request->header), sizeof(request->header), &io_error);
    if(io_error) {
        return io_error;
    }

    io__->Send(sd_, (void*) request->data.get(), request->header.data_size, &io_error);
    if(io_error) {
        return io_error;
    }

    return nullptr;
}

Error RequestHandlerTcp::ReceiveResponse() {
    Error err;
    SendDataResponse sendDataResponse;
    io__->Receive(sd_, &sendDataResponse, sizeof(sendDataResponse), &err);
    if(err != nullptr) {
        return err;
    }
    switch (sendDataResponse.error_code) {
    case kNetErrorFileIdAlreadyInUse :
        return ProducerErrorTemplates::kFileIdAlreadyInUse.Generate();
    case kNetAuthorizationError : {
        auto res_err = ProducerErrorTemplates::kAuthorizationFailed.Generate();
        res_err->Append(sendDataResponse.message);
        return res_err;
    }
    case kNetErrorNoError :
        return nullptr;
    default:
        return ProducerErrorTemplates::kInternalServerError.Generate();
    }
}

Error RequestHandlerTcp::TrySendToReceiver(const Request* request) {
    auto err = SendHeaderAndData(request);
    if (err)  {
        return err;
    }

    err = ReceiveResponse();
    if (err)  {
        return err;
    }

    log__->Debug(std::string("successfully sent data ") + " id: " + std::to_string(request->header.data_id) + " to " +
                 connected_receiver_uri_);
    return nullptr;
}


void RequestHandlerTcp::UpdateIfNewConnection() {
    if (Connected())
        return;
    UpdateReceiversList();
    (*ncurrent_connections_)++;
}

bool RequestHandlerTcp::UpdateReceiversList() {
    auto thread_receivers_new = discovery_service__->RotatedUriList(thread_id_);
    last_receivers_uri_update_ = high_resolution_clock::now();
    if (thread_receivers_new != receivers_list_) {
        receivers_list_ = thread_receivers_new;
        return true;
    }
    return false;
}

bool RequestHandlerTcp::TimeToUpdateReceiverList() {
    uint64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>( high_resolution_clock::now() -
                          last_receivers_uri_update_).count();
    return elapsed_ms > discovery_service__->UpdateFrequency();
}


bool RequestHandlerTcp::Disconnected() {
    return !Connected();
}


bool RequestHandlerTcp::NeedRebalance() {
    if (Disconnected())
        return false;

    if (TimeToUpdateReceiverList()) {
        return UpdateReceiversList();
    }
    return false;
}

void RequestHandlerTcp::CloseConnectionToPeformRebalance() {
    io__->CloseSocket(sd_, nullptr);
    log__->Debug("rebalancing");
    sd_ = kDisconnectedSocketDescriptor;
}

void RequestHandlerTcp::Disconnect() {
    io__->CloseSocket(sd_, nullptr);
    sd_ = kDisconnectedSocketDescriptor;
    log__->Debug("disconnected from  " + connected_receiver_uri_);
    connected_receiver_uri_.clear();
}

bool RequestHandlerTcp::ServerError(const Error& err) {
    return err != nullptr && err != ProducerErrorTemplates::kFileIdAlreadyInUse;
}

Error RequestHandlerTcp::ProcessRequestUnlocked(const Request* request) {
    if (NeedRebalance()) {
        CloseConnectionToPeformRebalance();
    }
    for (auto receiver_uri : receivers_list_) {
        if (Disconnected()) {
            auto err = ConnectToReceiver(request->beamtime_id, receiver_uri);
            if (err != nullptr ) continue;
        }

        auto err = TrySendToReceiver(request);
        if (ServerError(err))  {
            Disconnect();
            log__->Debug("cannot send data id " + std::to_string(request->header.data_id) + " to " + receiver_uri + ": " +
                         err->Explain());
            continue;
        }

        if (request->callback) {
            request->callback(request->header, std::move(err));
        }
        return nullptr;
    }
    return ProducerErrorTemplates::kCannotSendDataToReceivers.Generate();
}

bool RequestHandlerTcp::Connected() {
    return sd_ != kDisconnectedSocketDescriptor;
}

bool RequestHandlerTcp::CanCreateNewConnections() {
    return (*ncurrent_connections_) < discovery_service__->MaxConnections();
}

bool RequestHandlerTcp::ReadyProcessRequest() {
    return Connected() || CanCreateNewConnections();
}

void RequestHandlerTcp::PrepareProcessingRequestLocked() {
    UpdateIfNewConnection();
}

void RequestHandlerTcp::TearDownProcessingRequestLocked(const Error& error_from_process) {
    if (error_from_process) {
        (*ncurrent_connections_)--;
    }

}

}