#include "producer/producer_error.h"
#include "request_handler_tcp.h"
#include "producer_logger.h"
#include "io/io_factory.h"


namespace asapo {


RequestHandlerTcp::RequestHandlerTcp(ReceiverDiscoveryService* discovery_service,uint64_t thread_id):
    io__{GenerateDefaultIO()}, log__{GetDefaultProducerLogger()},discovery_service__{discovery_service},thread_id_{thread_id} {

}

Error RequestHandlerTcp::ConnectToReceiver(const std::string& receiver_address) {
    Error err;
    sd_ = io__->CreateAndConnectIPTCPSocket(receiver_address, &err);
    if(err != nullptr) {
        //log__->Debug("cannot connect to receiver at " + receiver_address + " - " + err->Explain());
        return err;
    }
    log__->Info("connected to receiver at " + receiver_address);
    return nullptr;
}

Error RequestHandlerTcp::SendHeaderAndData(const Request* request,const std::string& receiver_address) {
    Error io_error;
    io__->Send(sd_, &(request->header), sizeof(request->header), &io_error);
    if(io_error) {
// todo: add meaningful message to the io_error (here and below)
        log__->Debug("cannot send header to " + receiver_address + " - " + io_error->Explain());
        return io_error;
    }

    io__->Send(sd_, request->data, request->header.data_size, &io_error);
    if(io_error) {
        log__->Debug("cannot send data to " + receiver_address + " - " + io_error->Explain());
        return io_error;
    }

    return nullptr;
}

Error RequestHandlerTcp::ReceiveResponse(const std::string& receiver_address) {
    Error err;
    SendDataResponse sendDataResponse;
    io__->Receive(sd_, &sendDataResponse, sizeof(sendDataResponse), &err);
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

Error RequestHandlerTcp::TrySendToReceiver(const Request* request,const std::string& receiver_address) {
    auto err = SendHeaderAndData(request ,receiver_address);
    if (err)  {
        return err;
    }

    err = ReceiveResponse(receiver_address);
    if (err)  {
        return err;
    }

    log__->Debug("successfully sent data to " + receiver_address);
    return nullptr;
}


void RequestHandlerTcp::UpdateIfNewConnection() {
    if (sd_ != kDisconnectedSocketDescriptor)
        return;

    receivers_list_ = discovery_service__->RotatedUriList(thread_id_);
    last_rebalance_ = high_resolution_clock::now();
    ncurrent_connections_++;
}

bool RequestHandlerTcp::CheckForRebalance() {
    if (sd_ == kDisconnectedSocketDescriptor)
        return false;

    auto now =  high_resolution_clock::now();
    uint64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>( now -
        last_rebalance_).count();
    bool rebalance = false;
    if (elapsed_ms > discovery_service__->UpdateFrequency()) {
        auto thread_receivers_new = discovery_service__->RotatedUriList(thread_id_);
        last_rebalance_ = now;
        if (thread_receivers_new != receivers_list_) {
            receivers_list_ = thread_receivers_new;
            rebalance = true;
        }
    }
    return rebalance;

}

Error RequestHandlerTcp::ProcessRequestUnlocked(const Request* request){
    bool rebalance = CheckForRebalance();
    if (rebalance && sd_ != kDisconnectedSocketDescriptor) {
        io__->CloseSocket(sd_, nullptr);
        log__->Info("rebalancing");
        sd_ = kDisconnectedSocketDescriptor;
    }
    for (auto receiver_uri : receivers_list_) {
        if (sd_ == kDisconnectedSocketDescriptor) {
            auto err = ConnectToReceiver(receiver_uri);
            if (err != nullptr ) continue;
        }

        auto err = TrySendToReceiver(request,receiver_uri);
        if (err != nullptr && err != ProducerErrorTemplates::kFileIdAlreadyInUse)  {
            io__->CloseSocket(sd_, nullptr);
            sd_ = kDisconnectedSocketDescriptor;
            continue;
        }

        if (request->callback) {
            request->callback(request->header, std::move(err));
        }
        return nullptr;

    }
    return ProducerErrorTemplates::kCannotSendDataToReceivers.Generate();
}

bool RequestHandlerTcp::IsConnected() {
    return sd_ != kDisconnectedSocketDescriptor;
}

bool RequestHandlerTcp::CanCreateNewConnections() {
    return ncurrent_connections_ < discovery_service__->MaxConnections();
}

bool RequestHandlerTcp::ReadyProcessRequest() {
    return IsConnected() || CanCreateNewConnections();
}

void RequestHandlerTcp::PrepareProcessingRequestLocked() {
    UpdateIfNewConnection();
}

void RequestHandlerTcp::TearDownProcessingRequestLocked(const Error &error_from_process) {
    ncurrent_connections_--;
}

}
