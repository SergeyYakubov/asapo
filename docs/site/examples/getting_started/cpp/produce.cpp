#include "asapo/asapo_producer.h"

#include <iostream>

void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err) {
        std::cerr << "error/warning during send: " << err << std::endl;
        return;
    } else {
        std::cout << "successfuly send " << payload.original_header.Json() << std::endl;
        return;
    }
}

void exit_if_error(std::string error_string, const asapo::Error& err) {
    if (err) {
        std::cerr << error_string << err << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    asapo::Error err;

    auto endpoint = "localhost:8400"; // or your endpoint
    auto beamtime = "asapo_test";

    auto producer = asapo::Producer::Create(endpoint, 1, asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{asapo::SourceType::kProcessed, beamtime, "", "test_source", ""}, 60000, &err);
    exit_if_error("Cannot start producer", err);

    std::string to_send = "hello";
    auto send_size = to_send.size() + 1;
    auto buffer =  asapo::MessageData(new uint8_t[send_size]);
    memcpy(buffer.get(), to_send.c_str(), send_size);

    asapo::MessageHeader message_header{1, send_size, "processed/test_file"};
    err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
    exit_if_error("Cannot send message", err);

    err = producer->WaitRequestsFinished(2000);
    exit_if_error("Producer exit on timeout", err);

    return EXIT_SUCCESS;
}
