#include "producer/producer_error.h"
#include "request_handler_tcp.h"
#include "producer_logger.h"
#include "io/io_factory.h"
#include "producer_request.h"

namespace asapo {

RequestHandlerTcp::RequestHandlerTcp(ReceiverDiscoveryService* discovery_service, uint64_t thread_id,
                                     uint64_t* shared_counter):
    io__{GenerateDefaultIO()}, log__{GetDefaultProducerLogger()}, discovery_service__{discovery_service}, thread_id_{thread_id},
    ncurrent_connections_{shared_counter} {
}

Error RequestHandlerTcp::Authorize(const std::string& source_credentials) {
    GenericRequestHeader header{kOpcodeAuthorize, 0, 0, 0, source_credentials.c_str()};
    Error err;
    io__->Send(sd_, &header, sizeof(header), &err);
    if(err) {
        return err;
    }
    return ReceiveResponse(header);
}


Error RequestHandlerTcp::ConnectToReceiver(const std::string& source_credentials, const std::string& receiver_address) {
    Error err;

    sd_ = io__->CreateAndConnectIPTCPSocket(receiver_address, &err);
    if(err != nullptr) {
        log__->Debug("cannot connect to receiver at " + receiver_address + " - " + err->Explain());
        return err;
    }
    log__->Debug("connected to receiver at " + receiver_address);

    connected_receiver_uri_ = receiver_address;
    err = Authorize(source_credentials);
    if (err != nullptr) {
        log__->Error("authorization failed at " + receiver_address + " - " + err->Explain());
        Disconnect();
        return err;
    }

    log__->Info("authorized connection to receiver at " + receiver_address);

    return nullptr;
}

Error RequestHandlerTcp::SendRequestContent(const ProducerRequest* request) {
    Error io_error;
    io__->Send(sd_, &(request->header), sizeof(request->header), &io_error);
    if(io_error) {
        return io_error;
    }


    if (request->NeedSendMetaData()) {
        io__->Send(sd_, (void*) request->metadata.c_str(), (size_t) request->header.meta_size, &io_error);
        if (io_error) {
            return io_error;
        }
    }

    if (request->NeedSendData()) {
        if (request->DataFromFile()) {
            io_error = io__->SendFile(sd_,  request->original_filepath, (size_t)request->header.data_size);
        } else {
            io__->Send(sd_, (void*) request->data.get(), (size_t)request->header.data_size, &io_error);
        }
        if (io_error) {
            return io_error;
        }
    }

    return nullptr;
}

Error RequestHandlerTcp::ReceiveResponse(const GenericRequestHeader& request_header) {
    Error err;
    SendDataResponse sendDataResponse;
    io__->Receive(sd_, &sendDataResponse, sizeof(sendDataResponse), &err);
    if(err != nullptr) {
        return err;
    }

    switch (sendDataResponse.error_code) {
    case kNetAuthorizationError : {
        auto res_err = ProducerErrorTemplates::kWrongInput.Generate();
        res_err->Append(sendDataResponse.message);
        return res_err;
    }
    case kNetErrorWrongRequest : {
        auto res_err = ProducerErrorTemplates::kWrongInput.Generate();
        res_err->Append(sendDataResponse.message);
        return res_err;
    }
    case kNetErrorWarning: {
        auto res_err = ProducerErrorTemplates::kServerWarning.Generate();
        res_err->Append(sendDataResponse.message);
        return res_err;
    }
    case kNetErrorReauthorize: {
        auto res_err = ProducerErrorTemplates::kReAuthorizationNeeded.Generate();
        return res_err;
    }
    case kNetErrorNoError :
        return nullptr;
    default:
        auto res_err = ProducerErrorTemplates::kInternalServerError.Generate();
        res_err->Append(sendDataResponse.message);
        return res_err;
    }
}

Error RequestHandlerTcp::TrySendToReceiver(const ProducerRequest* request) {
    auto err = SendRequestContent(request);
    if (err)  {
        return err;
    }

    err = ReceiveResponse(request->header);
    if (err == nullptr || err == ProducerErrorTemplates::kServerWarning)  {
        log__->Debug("successfully sent data, opcode: " + std::to_string(request->header.op_code) +
                     ", id: " + std::to_string(request->header.data_id) + " to " + connected_receiver_uri_);
        if (err == ProducerErrorTemplates::kServerWarning ) {
            log__->Warning("warning from server for id " + std::to_string(request->header.data_id) + ": " + err->Explain());
        }
    }

    return err;
}


void RequestHandlerTcp::UpdateIfNewConnection() {
    if (Connected())
        return;
    UpdateReceiversList();
    (*ncurrent_connections_)++;
}

bool RequestHandlerTcp::UpdateReceiversList() {
    auto thread_receivers_new = discovery_service__->RotatedUriList(thread_id_);
    last_receivers_uri_update_ = system_clock::now();
    if (thread_receivers_new != receivers_list_) {
        receivers_list_ = thread_receivers_new;
        return true;
    }
    return false;
}

bool RequestHandlerTcp::TimeToUpdateReceiverList() {
    uint64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>( system_clock::now() -
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
    return err != nullptr && (err != ProducerErrorTemplates::kWrongInput &&
                              err != ProducerErrorTemplates::kLocalIOError &&
                              err != ProducerErrorTemplates::kServerWarning
                             );
}

bool RequestHandlerTcp::ProcessErrorFromReceiver(const Error& error,
                                                 const ProducerRequest* request,
                                                 const std::string& receiver_uri) {
    bool is_server_error = ServerError(error);

    if (error && error != ProducerErrorTemplates::kServerWarning) {
        Disconnect();
        std::string log_str = "cannot send data, opcode: " + std::to_string(request->header.op_code) +
                              ", id: " + std::to_string(request->header.data_id) + " to " + receiver_uri + ": " +
                              error->Explain();
        if (is_server_error) {
            log__->Warning(log_str + ", will try again");
        } else {
            log__->Error(log_str + ", request removed from queue");
        }
    }

    return is_server_error;
}


void RequestHandlerTcp::ProcessRequestCallback(Error err, ProducerRequest* request, bool* retry) {
    if (request->callback) {
        request->callback(request->header, std::move(err));
    }
    *retry = false;
}


bool RequestHandlerTcp::SendDataToOneOfTheReceivers(ProducerRequest* request, bool* retry) {
    for (auto receiver_uri : receivers_list_) {
        if (Disconnected()) {
            auto err = ConnectToReceiver(request->source_credentials, receiver_uri);
            if (err == ProducerErrorTemplates::kWrongInput) {
                ProcessRequestCallback(std::move(err), request, retry);
                return false;
            } else {
                if (err != nullptr ) continue;
            }
        }

        auto err = TrySendToReceiver(request);
        bool server_error_can_retry = ProcessErrorFromReceiver(err, request, receiver_uri);
        if (server_error_can_retry)  {
            continue;
        }

        bool success = err && err != ProducerErrorTemplates::kServerWarning ? false : true;
        ProcessRequestCallback(std::move(err), request, retry);
        return success;
    }
    log__->Warning("put back to the queue, request opcode: " + std::to_string(request->header.op_code) +
                   ", id: " + std::to_string(request->header.data_id));
    *retry = true;
    return false;
}


bool RequestHandlerTcp::ProcessRequestUnlocked(GenericRequest* request, bool* retry) {
    auto producer_request = static_cast<ProducerRequest*>(request);

    auto err = producer_request->UpdateDataSizeFromFileIfNeeded(io__.get());
    if (err) {
        if (producer_request->callback) {
            producer_request->callback(producer_request->header, std::move(err));
        }
        *retry = false;
        return false;
    }


    if (NeedRebalance()) {
        CloseConnectionToPeformRebalance();
    }
    return SendDataToOneOfTheReceivers(producer_request, retry);
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

void RequestHandlerTcp::TearDownProcessingRequestLocked(bool request_processed_successfully) {
    if (!request_processed_successfully) {
        (*ncurrent_connections_)--;
    }
}

void RequestHandlerTcp::ProcessRequestTimeout(GenericRequest* request) {
    auto producer_request = static_cast<ProducerRequest*>(request);

    log__->Error("request timeout, id:" + std::to_string(request->header.data_id) + " to " + request->header.substream +
                 " substream");

    auto err = ProducerErrorTemplates::kTimeout.Generate();
    if (producer_request->callback) {
        producer_request->callback(request->header, std::move(err));
    }

}

}
