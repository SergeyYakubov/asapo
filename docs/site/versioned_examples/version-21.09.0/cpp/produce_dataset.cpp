#include "asapo/asapo_producer.h"
#include <iostream>

void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err && err != asapo::ProducerErrorTemplates::kServerWarning) {
        std::cerr << "error during send: " << err << std::endl;
        return;
    } else if (err) {
        std::cout << "warning during send: " << err << std::endl;
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

    auto endpoint = "localhost:8400";
    auto beamtime = "asapo_test";

    auto credentials = asapo::SourceCredentials{asapo::SourceType::kProcessed, beamtime, "", "test_source", ""};
    
    auto producer = asapo::Producer::Create(endpoint, 1, asapo::RequestHandlerType::kTcp, credentials, 60000, &err);
    exit_if_error("Cannot start producer", err);

    std::string to_send = "hello dataset 1";
    auto send_size = to_send.size() + 1;
    auto buffer =  asapo::MessageData(new uint8_t[send_size]);
    memcpy(buffer.get(), to_send.c_str(), send_size);

    // add the additional paremeters to the header: part number in the dataset and the total number of parts
    asapo::MessageHeader message_header{1, send_size, "processed/test_file_dataset_1", "", 1, 3};

    err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
    exit_if_error("Cannot send message", err);

    // this can be done from different producers in any order
    // we do not recalculate send_size since we know it to be the same
    // we reuse the header to shorten the code
    to_send = "hello dataset 2";
    buffer =  asapo::MessageData(new uint8_t[send_size]);
    memcpy(buffer.get(), to_send.c_str(), send_size);

    message_header.dataset_substream = 2;
    err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
    exit_if_error("Cannot send message", err);

    to_send = "hello dataset 3";
    buffer =  asapo::MessageData(new uint8_t[send_size]);
    memcpy(buffer.get(), to_send.c_str(), send_size);

    message_header.dataset_substream = 3;
    err = producer->Send(message_header, std::move(buffer), asapo::kDefaultIngestMode, "default", &ProcessAfterSend);
    exit_if_error("Cannot send message", err);

    err = producer->WaitRequestsFinished(2000);
    exit_if_error("Producer exit on timeout", err);

    // the dataset parts are not counted towards the number of messages in the stream
    // the last message id in this example is still 1
    err = producer->SendStreamFinishedFlag("default", 1, "", &ProcessAfterSend);
    exit_if_error("Cannot finish stream", err);

    return EXIT_SUCCESS;
}
